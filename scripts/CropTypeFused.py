#!/usr/bin/python

from __future__ import print_function

import os
import os.path
import shutil
import argparse
import re
from lxml import etree
from lxml.builder import E
from sen2agri_common import ProcessorBase, Step, split_features, run_step, format_otb_filename, prepare_lut, save_lut


class CropTypeProcessor(ProcessorBase):

    def create_context(self):
        parser = argparse.ArgumentParser(description='Crop Type Processor')

        parser.add_argument('-refp', help='The reference polygons',
                            required=True, metavar='reference_polygons')
        parser.add_argument('-ratio', help='The ratio between the validation and training polygons (default 0.75)',
                            required=False, metavar='sample_ratio', default=0.75)
        parser.add_argument('-input', help='The list of products descriptors',
                            required=True, metavar='product_descriptor', nargs='+')
        parser.add_argument('-trm', help='The temporal resampling mode (default resample)',
                            choices=['resample', 'gapfill'], required=False, default='resample')
        parser.add_argument('-classifier', help='The classifier (rf or svm) used for training (default rf)',
                            required=False, metavar='classifier', choices=['rf', 'svm'], default='rf')
        parser.add_argument('-nbtrsample', help='The number of samples included in the training set (default 10000)',
                            required=False, metavar='nbtrsample', default=10000)
        parser.add_argument('-rseed', help='The random seed used for training (default 0)',
                            required=False, metavar='random_seed', default=0)
        parser.add_argument('-mask', help='The crop mask for each tile',
                            required=False, nargs='+', default=None)
        parser.add_argument('-maskprod', help='A crop mask product for the same tiles', required=False, default=None)
        parser.add_argument('-pixsize', help='The size, in meters, of a pixel (default 10)',
                            required=False, metavar='pixsize', default=10)
        parser.add_argument('-red-edge', help='Include Sentinel-2 vegetation red edge bands',
                            required=False, dest='red_edge', action='store_true')
        parser.add_argument('-no-red-edge', help='Don\'t include Sentinel-2 vegetation red edge bands',
                            required=False, dest='red_edge', action='store_false')
        parser.set_defaults(red_edge=True)
        parser.add_argument('-sp', help='Per-sensor sampling rates (default SENTINEL 10 SPOT 5 LANDSAT 16)',
                            required=False, nargs='+', default=["SENTINEL", "10", "SPOT", "5", "LANDSAT", "16"])

        parser.add_argument('-outdir', help="Output directory", default=".")
        parser.add_argument('-buildfolder', help="Build folder", default="")
        parser.add_argument(
            '-targetfolder', help="The folder where the target product is built", default="")

        parser.add_argument('-rfnbtrees', help='The number of trees used for training (default 100)',
                            required=False, metavar='rfnbtrees', default=100)
        parser.add_argument('-rfmax', help='maximum depth of the trees used for Random Forest classifier (default 25)',
                            required=False, metavar='rfmax', default=25)
        parser.add_argument('-rfmin', help='minimum number of samples in each node used by the classifier (default 25)',
                            required=False, metavar='rfmin', default=25)

        parser.add_argument('-keepfiles', help="Keep all intermediate files (default false)",
                            default=False, action='store_true')
        parser.add_argument('-siteid', help='The site ID', required=False, default='nn')
        parser.add_argument('-lut', help='Color LUT for previews (see /usr/share/sen2agri/crop-type.lut)', required=False)
        parser.add_argument('-outprops', help='Output properties file', required=False)

        parser.add_argument(
            '-strata', help='Shapefiles with polygons for the strata')
        parser.add_argument('-mode', help='The execution mode',
                            required=False, choices=['prepare-site', 'prepare-tiles', 'train', 'classify', 'postprocess-tiles', 'compute-quality-flags', 'validate'], default=None)
        parser.add_argument('-stratum-filter', help='The list of strata to use in training and classification',
                            required=False, type=int, nargs='+', default=None)
        parser.add_argument('-tile-filter', help='The list of tiles to apply the classification to',
                            required=False, nargs='+', default=None)
        parser.add_argument('-skip-quality-flags', help="Skip quality flags extraction, (default false)", default=False, action='store_true')
        parser.add_argument('-include-raw-map', help="Include the unmasked crop type map even when a crop mask was used (default true)", required=False, dest='include_raw_map', action='store_true')
        parser.add_argument('-no-include-raw-map', help="Don't include the unmasked crop type map when a crop mask was used", required=False, dest='include_raw_map', action='store_false')
        parser.set_defaults(include_raw_map=True)
        parser.add_argument('-max-parallelism', help="Number of tiles to process in parallel", required=False, type=int)
        parser.add_argument('-tile-threads-hint', help="Number of threads to use for a single tile, except for the post-filtering step (default 4)", required=False, type=int, default=4)
        self.args = parser.parse_args()

        self.args.lut = self.get_lut_path()

        if self.args.mask is not None and self.args.maskprod is not None:
            raise("The -mask and -maskprod arguments are exclusive")

    def load_tiles(self):
        super(CropTypeProcessor, self).load_tiles()

        for tile in self.tiles:
            tile.crop_mask = None

        if self.args.mask is not None:
            for idx, tile in self.tiles:
                if self.args.mask[idx] != 'NONE':
                    tile.crop_mask = self.args.mask[idx]
        elif self.args.maskprod is not None:
            mask_dict = {}
            tile_path = os.path.join(self.args.maskprod, 'TILES')
            tile_id_re = re.compile('_T([a-zA-Z0-9]+)$')

            for tile_dir in os.listdir(tile_path):
                m = tile_id_re.search(tile_dir)
                if m:
                    tile_id = m.group(1)
                    segmented_mask = None
                    raw_mask = None

                    img_data_path = os.path.join(os.path.join(tile_path, tile_dir), 'IMG_DATA')
                    for file in os.listdir(img_data_path):
                        if file.startswith('S2AGRI_L4A_CM'):
                            segmented_mask = os.path.join(img_data_path, file)
                        elif file.startswith('S2AGRI_L4A_RAW'):
                            raw_mask = os.path.join(img_data_path, file)

                    if segmented_mask is not None:
                        mask_dict[tile_id] = segmented_mask
                    elif raw_mask is not None:
                        mask_dict[tile_id] = raw_mask

            for tile in self.tiles:
                mask = mask_dict.get(tile.id)
                if mask is not None:
                    tile.crop_mask = mask

        for tile in self.tiles:
            if tile.crop_mask is not None:
                print("Crop mask for tile {}: {}".format(tile.id, tile.crop_mask))

    def prepare_site(self):
        if self.args.lut is not None:
            qgis_lut = self.get_output_path("qgis-color-map.txt")

            lut = prepare_lut(self.args.refp, self.args.lut)
            save_lut(lut, qgis_lut)

    def train_stratum(self, stratum):
        features_shapefile = self.get_output_path("features-{}.shp", stratum.id)

        split_features(stratum, self.args.refp, self.args.outdir)

        area_training_polygons = self.get_output_path("training_polygons-{}.shp", stratum.id)
        area_validation_polygons = self.get_output_path("validation_polygons-{}.shp", stratum.id)
        area_statistics = self.get_output_path("statistics-{}.xml", stratum.id)
        area_model = self.get_output_path("model-{}.txt", stratum.id)
        area_confmatout = self.get_output_path("confusion-matrix-training-{}.csv", stratum.id)
        area_days = self.get_output_path("days-{}.txt", stratum.id)

        area_descriptors = []
        area_prodpertile = []
        for tile in stratum.tiles:
            area_descriptors += tile.get_descriptor_paths()
            area_prodpertile.append(len(tile.descriptors))

        run_step(Step("SampleSelection", ["otbcli", "SampleSelectionAgri", self.args.buildfolder,
                                          "-ref", features_shapefile,
                                          "-ratio", self.args.ratio,
                                          "-seed", self.args.rseed,
                                          "-tp", area_training_polygons,
                                          "-vp", area_validation_polygons]))
        step_args = ["otbcli", "CropTypeTrainImagesClassifier", self.args.buildfolder,
                     "-mission", self.args.mission.name,
                     "-nodatalabel", -10000,
                     "-pixsize", self.args.pixsize,
                     "-outdays", area_days,
                     "-mode", self.args.trm,
                     "-io.vd", area_training_polygons,
                     "-rand", self.args.rseed,
                     "-sample.bm", 0,
                     "-io.confmatout", area_confmatout,
                     "-io.out", area_model,
                     "-sample.mt", self.args.nbtrsample,
                     "-sample.mv", 10,
                     "-sample.vfn", "CODE",
                     "-sample.vtr", 0.01,
                     "-classifier", self.args.classifier]
        if self.args.red_edge:
            step_args += ["-rededge", "true"]
        step_args += ["-sp"] + self.args.sp
        step_args += ["-prodpertile"] + area_prodpertile
        step_args += ["-il"] + area_descriptors
        if self.args.classifier == "rf":
            step_args += ["-classifier.rf.nbtrees", self.args.rfnbtrees,
                          "-classifier.rf.min", self.args.rfmin,
                          "-classifier.rf.max", self.args.rfmax]
        else:
            step_args += ["-classifier.svm.k", "rbf",
                          "-classifier.svm.opt", 1,
                          "-imstat", area_statistics]
        run_step(Step("TrainImagesClassifier", step_args))

    def classify_tile(self, tile):
        models = []
        model_ids = []
        days = []
        statistics = []
        for stratum in tile.strata:
            area_model = self.get_output_path("model-{}.txt", stratum.id)
            area_days = self.get_output_path("days-{}.txt", stratum.id)
            area_statistics = self.get_output_path("statistics-{}.xml", stratum.id)

            models.append(area_model)
            model_ids.append(stratum.id)
            days.append(area_days)
            statistics.append(area_statistics)

        if len(models) == 0:
            print("Skipping classification for tile {} due to stratum filter".format(tile.id))
            return

        if not self.single_stratum:
            tile_model_mask = self.get_output_path("model-mask-{}.tif", tile.id)

            run_step(Step("Rasterize model mask",
                          ["otbcli_Rasterization",
                           "-progress", "false",
                           "-mode", "attribute",
                           "-mode.attribute.field", "ID",
                           "-in", self.args.filtered_strata,
                           "-im", tile.reference_raster,
                           "-out", format_otb_filename(tile_model_mask, compression='DEFLATE'), "uint8"]))

        tile_crop_type_map_uncompressed = self.get_output_path("crop_type_map_{}_uncompressed.tif", tile.id)

        step_args = ["otbcli", "CropTypeImageClassifier", self.args.buildfolder,
                     "-progress", "false",
                     "-mission", self.args.mission.name,
                     "-pixsize", self.args.pixsize,
                     "-bv", -10000,
                     "-nodatalabel", -10000,
                     "-out", tile_crop_type_map_uncompressed,
                     "-indays"] + days
        if self.args.red_edge:
            step_args += ["-rededge", "true"]
        step_args += ["-model"] + models
        step_args += ["-il"] + tile.get_descriptor_paths()
        if self.args.classifier == "svm":
            step_args += ["-imstat"] + statistics

        if not self.single_stratum:
            step_args += ["-mask", tile_model_mask]
            step_args += ["-modelid"] + model_ids

        run_step(Step("ImageClassifier_{}".format(tile.id), step_args, retry=True))

        if not self.args.keepfiles and not self.single_stratum:
            os.remove(tile_model_mask)

        tile_crop_type_map = self.get_tile_classification_output(tile)

        os.system('gdal_translate -of GTiff -co "COMPRESS=DEFLATE" -ot Int16 ' +
                tile_crop_type_map_uncompressed + ' ' + tile_crop_type_map)
        """
        step_args = ["otbcli_Convert",
                     "-progress", "false",
                     "-in", tile_crop_type_map_uncompressed,
                     "-out", format_otb_filename(tile_crop_type_map, compression='DEFLATE'), "int16"]
        run_step(Step("Compression_{}".format(tile.id), step_args))
        """

        if not self.args.keepfiles:
            os.remove(tile_crop_type_map_uncompressed)

    def postprocess_tile(self, tile):
        if tile.crop_mask is not None:
            tile_crop_map = self.get_output_path("crop_type_map_{}.tif", tile.id)
            tile_crop_map_masked = self.get_output_path("crop_type_map_masked_{}.tif", tile.id)

            step_args = ["otbcli_BandMath",
                         "-progress", "false",
                         "-exp", "im2b1 == 0 ? 0 : im1b1",
                         "-il", tile_crop_map, tile.crop_mask,
                         "-out", format_otb_filename(tile_crop_map_masked, compression='DEFLATE'), "int16"]

            run_step(Step("Mask by crop mask " + tile.id, step_args))

            run_step(Step("Nodata_" + tile.id,
                          ["gdal_edit.py",
                           "-a_nodata", -10000,
                           tile_crop_map_masked]))

    def get_tile_crop_map(self, tile):
        if tile.crop_mask is not None:
            return self.get_output_path("crop_type_map_masked_{}.tif", tile.id)
        else:
            return self.get_output_path("crop_type_map_{}.tif", tile.id)

    def validate(self, context):
        for stratum in self.strata:
            area_validation_polygons = self.get_output_path("validation_polygons-{}.shp", stratum.id)
            area_statistics = self.get_output_path("confusion-matrix-validation-{}.csv", stratum.id)
            area_quality_metrics = self.get_output_path("quality-metrics-{}.txt", stratum.id)
            area_validation_metrics_xml = self.get_output_path("validation-metrics-{}.xml", stratum.id)

            step_args = ["otbcli", "ComputeConfusionMatrixMulti", self.args.buildfolder,
                         "-ref", "vector",
                         "-ref.vector.in", area_validation_polygons,
                         "-ref.vector.field", "CODE",
                         "-out", area_statistics,
                         "-nodatalabel", -10000,
                         "-il"]
            for tile in stratum.tiles:
                step_args.append(self.get_tile_crop_map(tile))

            run_step(Step("ComputeConfusionMatrix_" + str(stratum.id),
                          step_args, out_file=area_quality_metrics))

            step_args = ["otbcli", "XMLStatistics", self.args.buildfolder,
                         "-root", "CropType",
                         "-confmat", area_statistics,
                         "-quality", area_quality_metrics,
                         "-out", area_validation_metrics_xml]
            run_step(Step("XMLStatistics_" + str(stratum.id), step_args))

        if not self.single_stratum:
            global_validation_metrics_xml = self.get_output_path("validation-metrics-global.xml")

            if len(self.strata) > 1:
                global_validation_polygons = self.get_output_path("validation_polygons_global.shp")
                global_prj_file = self.get_output_path("validation_polygons_global.prj")
                global_statistics = self.get_output_path("confusion-matrix-validation-global.csv")
                global_quality_metrics = self.get_output_path("quality-metrics-global.txt")

                files = []
                for stratum in self.strata:
                    area_validation_polygons = self.get_output_path("validation_polygons-{}.shp", stratum.id)

                    files.append(area_validation_polygons)

                step_args = ["otbcli_ConcatenateVectorData",
                             "-out", global_validation_polygons,
                             "-vd"] + files
                run_step(Step("ConcatenateVectorData", step_args))

                first_prj_file = self.get_output_path("validation_polygons-{}.prj", self.strata[0].id)
                shutil.copyfile(first_prj_file, global_prj_file)

                step_args = ["otbcli", "ComputeConfusionMatrixMulti", self.args.buildfolder,
                             "-ref", "vector",
                             "-ref.vector.in", global_validation_polygons,
                             "-ref.vector.field", "CODE",
                             "-out", global_statistics,
                             "-nodatalabel", -10000,
                             "-il"]
                for tile in self.tiles:
                    step_args.append(self.get_tile_crop_map(tile))

                run_step(Step("ComputeConfusionMatrix_Global",
                              step_args, out_file=global_quality_metrics))

                step_args = ["otbcli", "XMLStatistics", self.args.buildfolder,
                             "-root", "CropType",
                             "-confmat", global_statistics,
                             "-quality", global_quality_metrics,
                             "-out", global_validation_metrics_xml]
                run_step(Step("XMLStatistics_Global", step_args))
            else:
                area_validation_metrics_xml = self.get_output_path("validation-metrics-{}.xml", self.strata[0].id)

                shutil.copyfile(area_validation_metrics_xml, global_validation_metrics_xml)

        step_args = ["otbcli", "ProductFormatter", self.args.buildfolder,
                     "-destroot", self.args.targetfolder,
                     "-fileclass", "SVT1",
                     "-level", "L4B",
                     "-baseline", "01.00",
                     "-siteid", self.args.siteid,
                     "-gipp", self.get_metadata_file(),
                     "-isd", self.get_in_situ_data_file(),
                     "-processor", "croptype"]

        if self.args.lut is not None:
            qgis_lut = self.get_output_path("qgis-color-map.txt")

            step_args += ["-lut", self.args.lut,
                          "-lutqgis", qgis_lut]

        if self.args.outprops is not None:
            step_args += ["-outprops", self.args.outprops]

        has_mask = False
        for tile in self.tiles:
            if tile.crop_mask is not None:
                has_mask = True
                break

        step_args.append("-processor.croptype.file")
        for tile in self.tiles:
            tile_crop_map = self.get_tile_crop_map(tile)

            step_args.append("TILE_" + tile.id)
            step_args.append(tile_crop_map)

        if has_mask and self.args.include_raw_map:
            step_args.append("-processor.croptype.rawfile")
            for tile in self.tiles:
                if tile.crop_mask is not None:
                    tile_crop_map = self.get_output_path("crop_type_map_{}.tif", tile.id)

                    step_args.append("TILE_" + tile.id)
                    step_args.append(tile_crop_map)

        step_args.append("-processor.croptype.flags")
        for tile in self.tiles:
            tile_quality_flags = self.get_output_path("status_flags_{}.tif", tile.id)

            step_args.append("TILE_" + tile.id)
            step_args.append(tile_quality_flags)

        step_args.append("-processor.croptype.quality")
        if not self.single_stratum:
            global_validation_metrics_xml = self.get_output_path("validation-metrics-global.xml")

            step_args.append(global_validation_metrics_xml)

        for stratum in self.strata:
            area_validation_metrics_xml = self.get_output_path("validation-metrics-{}.xml", stratum.id)

            if not self.single_stratum:
                step_args.append("REGION_" + str(stratum.id))
            step_args.append(area_validation_metrics_xml)

        step_args.append("-il")
        step_args += self.args.input

        run_step(Step("ProductFormatter", step_args))

    def get_tile_classification_output(self, tile):
        return self.get_output_path("crop_type_map_{}.tif", tile.id)

    def get_lut_path(self):
        if self.args.lut is not None:
            lut_path = self.args.lut
            if os.path.isfile(lut_path):
                return lut_path
            else:
                print("Warning: The LUT file {} does not exist, using the default one".format(self.args.lut))

        script_dir = os.path.dirname(__file__)
        lut_path = os.path.join(script_dir, "../sen2agri-processors/CropType/crop-type.lut")
        if not os.path.isfile(lut_path):
            lut_path = os.path.join(script_dir, "../share/sen2agri/crop-type.lut")
        if not os.path.isfile(lut_path):
            lut_path = os.path.join(script_dir, "crop-type.lut")
        if not os.path.isfile(lut_path):
            lut_path = os.path.join(script_dir, "/usr/share/sen2agri/crop-type.lut")
        if not os.path.isfile(lut_path):
            lut_path = None

        return lut_path

    def build_metadata(self):
        tiles = E.Tiles()
        for tile in self.tiles:
            inputs = E.Inputs()
            for descriptor in tile.get_descriptor_paths():
                inputs.append(
                    E.Input(os.path.splitext(os.path.basename(descriptor))[0])
                )

            tile_el = E.Tile(
                E.Id(tile.id),
                inputs
            )
            if tile.crop_mask is not None:
                tile_el.append(
                    E.Mask(os.path.splitext(os.path.basename(tile.crop_mask))[0])
                )

            tiles.append(tile_el)

        metadata = E.Metadata(
            E.ProductType("Crop Type"),
            E.Level("L4B"),
            E.SiteId(self.args.siteid),
            E.ReferencePolygons(os.path.basename(self.args.refp)))

        if self.args.strata is not None:
            metadata.append(
                E.Strata(os.path.basename(self.args.strata))
            )

        metadata.append(tiles)

        classifier = E.Classifier(
            E.TrainingSamplesPerTile(str(self.args.nbtrsample))
        )

        if self.args.classifier == 'rf':
            classifier.append(
                E.RF(
                    E.NbTrees(str(self.args.rfnbtrees)),
                    E.Min(str(self.args.rfmin)),
                    E.Max(str(self.args.rfmax))
                )
            )
        else:
            classifier.append(
                E.SVM()
            )

        parameters = E.Parameters(
            E.MainMission(self.args.mission.name),
            E.PixelSize(str(self.args.pixsize)),
            E.SampleRatio(str(self.args.ratio)),
            E.Classifier(self.args.classifier),
            E.Seed(str(self.args.rseed)),
            E.IncludeRedEdge(str(self.args.red_edge))
        )

        if self.args.lut is not None:
            parameters.append(
                E.LUT(os.path.basename(self.args.lut))
            )

        parameters.append(
            classifier
        )

        metadata.append(
            parameters
        )

        return etree.ElementTree(metadata)


processor = CropTypeProcessor()
processor.execute()

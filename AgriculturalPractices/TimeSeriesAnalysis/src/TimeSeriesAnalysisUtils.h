#ifndef TimeSeriesAnalysisUtils_h
#define TimeSeriesAnalysisUtils_h

#include <time.h>
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/algorithm/string.hpp"

#include <gsl/gsl_fit.h>
#include <gsl/gsl_cdf.h>

#include "CommonFunctions.h"

#define SEC_IN_DAY                   86400          // seconds in day = 24 * 3600
#define SEC_IN_WEEK                  604800         // seconds in week = 7 * 24 * 3600

#define DOUBLE_EPSILON                  0.00000001

template <typename T>
inline bool IsNA(T val) {
    return (val == NOT_AVAILABLE || val == NR || val == NOT_AVAILABLE_1);
}

inline bool IsEqual(const double &val1, const double &val2) {
    if (std::fabs(val1-val2) < DOUBLE_EPSILON) {
        return true;    // equal values
    }
    return false;
}

inline bool IsLessOrEqual(const double &val1, const double &val2) {
    if (std::fabs(val1-val2) < DOUBLE_EPSILON) {
        return true;    // equal values
    }
    return (val1 < val2);
}

inline bool IsGreaterOrEqual(const double &val1, const double &val2) {
    if (std::fabs(val1-val2) < DOUBLE_EPSILON) {
        return true;    // equal values
    }
    return (val1 > val2);
}

inline bool IsLess(const double &val1, const double &val2) {
    if (std::fabs(val1-val2) < DOUBLE_EPSILON) {
        return false;    // equal values
    }
    return (val1 < val2);
}

inline bool IsGreater(const double &val1, const double &val2) {
    if (std::fabs(val1-val2) < DOUBLE_EPSILON) {
        return false;    // equal values
    }
    return (val1 > val2);
}

inline boost::gregorian::date GetBoostDateFromString(const std::string &strDate) {
    return boost::gregorian::from_simple_string(strDate);
}

inline bool ComputeSlope(const std::vector<double>& x, const std::vector<double>& y, double &slope){
    if(x.size() != y.size()) {
        return false;
    }
    double n = x.size();

    double avgX = std::accumulate(x.begin(), x.end(), 0.0) / n;
    double avgY = std::accumulate(y.begin(), y.end(), 0.0) / n;

    double numerator = 0.0;
    double denominator = 0.0;

    for(int i=0; i<n; ++i){
        numerator += (x[i] - avgX) * (y[i] - avgY);
        denominator += (x[i] - avgX) * (x[i] - avgX);
    }

    if(denominator == 0){
        return false;
    }

    slope = numerator / denominator;

    return true;
}

//template <typename T>
//inline std::vector<T> GetLinearFit(const std::vector<T>& xData, const std::vector<T>& yData)
//{
//    T xSum = 0, ySum = 0, xxSum = 0, xySum = 0, slope, intercept;
//    for (long i = 0; i < yData.size(); i++)
//    {
//        xSum += xData[i];
//        ySum += yData[i];
//        xxSum += xData[i] * xData[i];
//        xySum += xData[i] * yData[i];
//    }
//    slope = (yData.size() * xySum - xSum * ySum) / (yData.size() * xxSum - xSum * xSum);
//    intercept = (ySum - slope * xSum) / yData.size();
//    std::vector<T> res;
//    res.push_back(slope);
//    res.push_back(intercept);
//    return res;
//}

template <typename T>
bool ComputePValue(const std::vector<T>& xData, const std::vector<T>& yData, double &pValue) {
    if(xData.size() != yData.size() || xData.size() <= 2) {
        return false;
    }

    const double* xData1 = &xData[0];
    const double* yData1 = &yData[0];
    int n = xData.size();
    double c0, c1, cov00, cov01, cov11, sumsq;
    gsl_fit_linear (xData1, 1, yData1, 1, n, &c0, &c1, &cov00, &cov01, &cov11, &sumsq);

    double dl=n-2;//degrees of liberty
    double ySum = 0;
    for (long i = 0; i < n; i++) {
        ySum += yData1[i];
    }
    double ym = ySum / n; //Average of vector y
    double sct = 0; // sct = sum of total squares
    for (long i = 0; i < n; i++) {
        sct += pow(yData1[i]-ym,2);
    }
    //double sct=pow(yData1[0]-ym,2)+pow(yData1[1]-ym,2)+pow(yData1[2]-ym,2)+pow(yData1[3]-ym,2); // sct = sum of total squares
    double R2=1-sumsq/sct;
    //std::cout<<"Multiple R-squared: "<<R2<<",    Adjusted R-squared: "<<1-double(n-1)/dl*(1-R2)<<std::endl;
    double F=R2*dl/(1-R2);
    double p_value=1-gsl_cdf_fdist_P(F,1,dl);
    //std::cout<<"F-statistic:  "<<F<<" on 1 and "<<n-2<<" DF,  p-value: "<<p_value<<std::endl;
    pValue = p_value;
    return true;
}

inline std::vector<std::string> LineToVector(const std::string &line, const std::string &separators = {';'})
{
    std::vector<std::string> results;
    if (separators.size() > 0) {
        boost::split(results,line,boost::is_any_of(separators));
    }
    return results;
}

inline int GetPosInVector(const std::vector<std::string> &vect, const std::string &item)
{
    auto result = std::find(vect.begin(), vect.end(), item);
    if (result != vect.end()) {
        return std::distance(vect.begin(), std::find(vect.begin(), vect.end(), item));
    }
    return -1;
}

inline bool ComputeMeanAndStandardDeviation (const std::vector<double> &inVect,
                                      double &meanVal, double &stdDevVal,
                                      bool genSampleStandardDev = false) {
    if (inVect.size() == 0) {
        return false;
    }
    if (genSampleStandardDev && inVect.size() == 1) {
        return false;
    }
    double sum = std::accumulate(inVect.begin(), inVect.end(), 0.0);
    meanVal = sum / inVect.size();

    // Boost implementation
//        boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::variance> > acc;
//        for (int i = 0; i<inVect.size(); i++) {
//            acc(inVect[i]);
//        }
//        //cout << mean(acc) << endl;
//        stdDevVal = sqrt(boost::accumulators::variance(acc));

    // Non boost implementation

    std::vector<double> diff(inVect.size());
    std::transform(inVect.begin(), inVect.end(), diff.begin(), [meanVal](double x) { return x - meanVal; });
    double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    int vecSize = inVect.size();
    if (genSampleStandardDev) {
        vecSize -= 1;
    }
    stdDevVal = std::sqrt(sq_sum / vecSize);

    return true;
}



#endif

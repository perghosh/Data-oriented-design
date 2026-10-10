// @FILE [tag: benchmark] [description: Benchmarking functions for GD library] [name: gd_benchmark.cpp] [type: source] 

#include "gd_benchmark.h"

_GD_BEGIN


argument::arguments benchmark::to_arguments_s(const result& result_)
{
   argument::arguments arguments_;
   arguments_.append("Iterations", result_.m_uIterations);
   arguments_.append("Mean (ns)", result_.m_dMean);
   arguments_.append("Min Time (ns)", result_.m_dMinTime);
   arguments_.append("Max Time (ns)", result_.m_dMaxTime);
   arguments_.append("Standard Deviation (ns)", result_.m_dStandardDeviation);
   arguments_.append("Total Duration (ns)", result_.m_dTotalDurationNs);
   return arguments_;
}


argument::arguments benchmark::to_arguments_print_s(const result& result_) 
{
   argument::arguments arguments_;
   arguments_.append("Iterations", result_.m_uIterations);
   arguments_.append("Mean (ns)", result_.m_dMean);
   arguments_.append("Min Time (ns)", result_.m_dMinTime);
   arguments_.append("Max Time (ns)", result_.m_dMaxTime);
   arguments_.append("Standard Deviation (ns)", result_.m_dStandardDeviation);
   arguments_.append("Total Duration (ns)", result_.m_dTotalDurationNs);
   return arguments_;
}

_GD_END
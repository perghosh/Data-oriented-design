/**
 * \file gd_log_logger_define.h
 * 
 * \brief macros used to simplify information sent to logger 
 * 
 * *Macros described*
 * - `LOG_` The core macro for all other macros used, here is where the actions is
 * - `LOG` Use this with the default logger, here you need to set all information that is sent to logger
 * - `LOG_NR` When multiple loggers are used, this is needed to log with other loggers than the 0 logger (the default logger)
 * - `LOG_IF_` Generate log if condition is true
 * - `LOG_IF` Use default logger 0 with condition if log or not
 * - `LOG_IF_NR` If log based on condition is true or not and multiple loggers are used
 * 
 * Other log macros are used to simplify or make code more readable, one macro type for
 * each severity level.
 */

#pragma once


#ifdef GD_LOG_SIMPLE

   // `LOG_` does it all, in the end all other log macros will call `LOG_`
   #define LOG_( uLogger, uSeverity, expression ) gd::log::get_g<uLogger,false>()->print( gd::log::message( gd::log::severity_get_g( uSeverity ), gd::log::eMessageTypeAll ) << __FILE__ << __func__ << expression )
   #define LOG2_( uLogger, uSeverity, tag, expression ) \
      if( (*gd::log::get_g<uLogger,false>())( tag ) ) gd::log::get_g<uLogger,false>()->print( gd::log::message( gd::log::severity_get_g( uSeverity ), gd::log::eMessageTypeAll ) << __FILE__ << __func__ << expression )
   // `LOG_RAW_` doesn't print file and function name, it only prints what is sent. Good to have when you only want to produce information
   //#define LOG_RAW_( uLogger, uSeverity, expression ) gd::log::get_g<uLogger,false>()->print( gd::log::message( gd::log::severity_get_g( uSeverity ), gd::log::eMessageTypeAll ) << expression )
   
   #define LOG_RAW_( uLogger, uSeverity, expression ) gd::log::print_message<uLogger,false>( gd::log::message( gd::log::severity_get_g( uSeverity ), gd::log::eMessageTypeAll ) << expression )
   #define LOG_RAW2_( uLogger, uSeverity, tag, expression ) \
      if( (*gd::log::get_g<uLogger,false>())( tag ) ) gd::log::print_message<uLogger,false>( gd::log::message( gd::log::severity_get_g( uSeverity ), gd::log::eMessageTypeAll ) << expression )

   
   #define LOG( uSeverity, expression ) LOG_( 0, gd::log::severity_get_g( uSeverity ), expression )
   #define LOG2( uSeverity, tag, expression ) LOG2_( 0, gd::log::severity_get_g( uSeverity ), tag, expression )
   #define LOG_NR( uNumber, uSeverity, expression ) LOG_( uNumber, gd::log::severity_get_g( uSeverity ), expression )
   #define LOG_RAW( uSeverity, expression ) LOG_RAW_( 0, gd::log::severity_get_g( uSeverity ), expression )
   #define LOG_RAW2( uSeverity, tag, expression ) LOG_RAW2_( 0, gd::log::severity_get_g( uSeverity ), tag, expression )

   #define LOG_SET_SEVERITY( uSeverity ) gd::log::get_g<0,false>()->set_severity( gd::log::severity_get_g( uSeverity ) )
   #define LOG_GET_SEVERITY() gd::log::get_g<0,false>()->get_severity()
   

   #define LOG_IF_(uLogger, uSeverity, condition, expression)  if(!(condition)) {;} else LOG_(uLogger, uSeverity, expression)
   #define LOG_IF(uSeverity, condition, expression)  LOG_IF_(0, uSeverity, condition, expression)
   #define LOG_IF_NR( uNumber, uSeverity, condition, expression)  LOG_IF_(uNumber, uSeverity, condition, expression)

   #define LOG_RAW_IF_(uLogger, uSeverity, condition, expression)  if(!(condition)) {;} else LOG_RAW_(uLogger, uSeverity, expression)
   #define LOG_RAW_IF(uSeverity, condition, expression)  LOG_RAW_IF_(0, uSeverity, condition, expression)
   
   #define LOG_FATAL(expression)                   LOG("FATAL", expression)
   #define LOG_ERROR(expression)                   LOG("ERROR", expression)
   #define LOG_WARNING(expression)                 LOG("WARNING", expression)
   #define LOG_INFORMATION(expression)             LOG("INFORMATION", expression)
   #define LOG_DEBUG(expression)                   LOG("DEBUG", expression)
   #define LOG_VERBOSE(expression)                 LOG("VERBOSE", expression)
   #define LOG_NONE(expression)                    LOG("NONE", expression)

   #define LOG_FATAL2(tag, expression)             LOG2("FATAL", tag, expression)
   #define LOG_ERROR2(tag, expression)             LOG2("ERROR", tag, expression)
   #define LOG_WARNING2(tag, expression)           LOG2("WARNING", tag, expression)
   #define LOG_INFORMATION2(tag, expression)       LOG2("INFORMATION", tag, expression)
   #define LOG_DEBUG2(tag, expression)             LOG2("DEBUG", tag, expression)
   #define LOG_VERBOSE2(tag, expression)           LOG2("VERBOSE", tag, expression)
   #define LOG_NONE2(tag, expression)              LOG2("NONE", tag, expression)

   #define LOG_FATAL_RAW(expression)               LOG_RAW("FATAL", expression)
   #define LOG_ERROR_RAW(expression)               LOG_RAW("ERROR", expression)
   #define LOG_WARNING_RAW(expression)             LOG_RAW("WARNING", expression)
   #define LOG_INFORMATION_RAW(expression)         LOG_RAW("INFORMATION", expression)
   #define LOG_DEBUG_RAW(expression)               LOG_RAW("DEBUG", expression)
   #define LOG_VERBOSE_RAW(expression)             LOG_RAW("VERBOSE", expression)
   #define LOG_NONE_RAW(expression)                LOG_RAW("NONE", expression)

   #define LOG_FATAL_RAW2(tag, expression)         LOG_RAW2("FATAL", tag, expression)
   #define LOG_ERROR_RAW2(tag, expression)         LOG_RAW2("ERROR", tag, expression)
   #define LOG_WARNING_RAW2(tag, expression)       LOG_RAW2("WARNING", tag, expression)
   #define LOG_INFORMATION_RAW2(tag, expression)   LOG_RAW2("INFORMATION", tag, expression)
   #define LOG_DEBUG_RAW2(tag, expression)         LOG_RAW2("DEBUG", tag, expression)
   #define LOG_VERBOSE_RAW2(tag, expression)       LOG_RAW2("VERBOSE", tag, expression)
   #define LOG_NONE_RAW2(tag, expression)          LOG_RAW2("NONE", tag, expression)

   #define LOG_FATAL_RAW_IF(condition,expression)     LOG_RAW_IF("FATAL", condition, expression)
   #define LOG_ERROR_RAW_IF(condition,expression)     LOG_RAW_IF("ERROR", condition, expression)
   #define LOG_WARNING_RAW_IF(condition,expression)   LOG_RAW_IF("WARNING", condition, expression)
   #define LOG_INFORMATION_RAW_IF(condition,expression) LOG_RAW_IF("INFORMATION", condition, expression)
   #define LOG_DEBUG_RAW_IF(condition,expression)     LOG_RAW_IF("DEBUG", condition, expression)
   #define LOG_VERBOSE_RAW_IF(condition,expression)   LOG_RAW_IF("VERBOSE", condition, expression)
   #define LOG_NONE_RAW_IF(condition,expression)      LOG_RAW_IF("NONE", condition, expression)

   #define LOG_FATAL_IF(condition, expression)        LOG_IF("FATAL", condition, expression)
   #define LOG_ERROR_IF(condition, expression)        LOG_IF("ERROR", condition, expression)
   #define LOG_WARNING_IF(condition, expression)      LOG_IF("WARNING", condition, expression)
   #define LOG_INFORMATION_IF(condition, expression)  LOG_IF("INFORMATION", condition, expression)
   #define LOG_DEBUG_IF(condition, expression)        LOG_IF("DEBUG", condition, expression)
   #define LOG_VERBOSE_IF(condition, expression)      LOG_IF("VERBOSE", condition, expression)
   #define LOG_NONE_IF(condition, expression)         LOG_IF("NONE", condition, expression)


   #define LOG_FATAL_NR(uNumber, expression)       LOG_NR(uNumber, "FATAL", expression)
   #define LOG_ERROR_NR(uNumber, expression)       LOG_NR(uNumber, "ERROR", expression)
   #define LOG_WARNING_NR(uNumber, expression)     LOG_NR(uNumber, "WARNING", expression)
   #define LOG_INFORMATION_NR(uNumber, expression) LOG_NR(uNumber, "INFORMATION", expression)
   #define LOG_DEBUG_NR( uNumber, expression)      LOG_NR(uNumber, "DEBUG", expression)
   #define LOG_VERBOSE_NR(uNumber, expression)     LOG_NR(uNumber, "VERBOSE", expression)
   #define LOG_NONE_NR(uNumber, expression)        LOG_NR(uNumber, "NONE", expression)

   #define LOG_FATAL_IF_NR( uNumber, condition, expression)        LOG_IF_NR(uNumber, "FATAL", condition, expression)
   #define LOG_ERROR_IF_NR( uNumber, condition, expression)        LOG_IF_NR(uNumber, "ERROR", condition, expression)
   #define LOG_WARNING_IF_NR( uNumber, condition, expression)      LOG_IF_NR(uNumber, "WARNING", condition, expression)
   #define LOG_INFORMATION_IF_NR( uNumber, condition, expression)  LOG_IF_NR(uNumber, "INFORMATION", condition, expression)
   #define LOG_DEBUG_IF_NR( uNumber, condition, expression)        LOG_IF_NR(uNumber, "DEBUG", condition, expression)
   #define LOG_VERBOSE_IF_NR( uNumber, condition, expression)      LOG_IF_NR(uNumber, "VERBOSE", condition, expression)
   #define LOG_NONE_IF_NR( uNumber, condition, expression)         LOG_IF_NR(uNumber, "NONE", condition, expression)

#else

   #define LOG_( uLogger, uSeverity, expression ) ((void)0)
   #define LOG2_( uLogger, uSeverity, tag, expression ) ((void)0)
   #define LOG_RAW_( uLogger, uSeverity, expression ) ((void)0)
   #define LOG_RAW2_( uLogger, uSeverity, tag, expression ) ((void)0)

   #define LOG( uSeverity, expression ) LOG_( 0, gd::log::severity_get_g( uSeverity ), expression )
   #define LOG_NR( uNumber, uSeverity, expression ) LOG_( uNumber, gd::log::severity_get_g( uSeverity ), expression )
   #define LOG_RAW( uSeverity, expression ) LOG_RAW_( 0, gd::log::severity_get_g( uSeverity ), expression )
   #define LOG_RAW2( uSeverity, tag, expression ) LOG_RAW2_( 0, gd::log::severity_get_g( uSeverity ), tag, expression )

   #define LOG_SET_SEVERITY( uSeverity ) ((void)0)
   #define LOG_GET_SEVERITY() 0

   #define LOG_IF_(uLogger, uSeverity, condition, expression)  if(!(condition)) {;} else LOG_(uLogger, uSeverity, expression)
   #define LOG_IF(uSeverity, condition, expression)  LOG_IF_(0, uSeverity, condition, expression)
   #define LOG_IF_NR( uNumber, uSeverity, condition, expression)  LOG_IF_(uNumber, uSeverity, condition, expression)

   #define LOG_RAW_IF_(uLogger, uSeverity, condition, expression)  if(!(condition)) {;} else LOG_RAW_(uLogger, uSeverity, expression)
   #define LOG_RAW_IF(uSeverity, condition, expression)  LOG_RAW_IF_(0, uSeverity, condition, expression)


   #define LOG_FATAL(expression)                   LOG("FATAL", expression)
   #define LOG_ERROR(expression)                   LOG("ERROR", expression)
   #define LOG_WARNING(expression)                 LOG("WARNING", expression)
   #define LOG_INFORMATION(expression)             LOG("INFORMATION", expression)
   #define LOG_DEBUG(expression)                   LOG("DEBUG", expression)
   #define LOG_VERBOSE(expression)                 LOG("VERBOSE", expression)
   #define LOG_NONE(expression)                    LOG("NONE", expression)

   #define LOG_FATAL2(tag, expression)             LOG2("FATAL", tag, expression)
   #define LOG_ERROR2(tag, expression)             LOG2("ERROR", tag, expression)
   #define LOG_WARNING2(tag, expression)           LOG2("WARNING", tag, expression)
   #define LOG_INFORMATION2(tag, expression)       LOG2("INFORMATION", tag, expression)
   #define LOG_DEBUG2(tag, expression)             LOG2("DEBUG", tag, expression)
   #define LOG_VERBOSE2(tag, expression)           LOG2("VERBOSE", tag, expression)
   #define LOG_NONE2(tag, expression)              LOG2("NONE", tag, expression)

   #define LOG_FATAL_RAW(expression)               LOG_RAW("FATAL", expression)
   #define LOG_ERROR_RAW(expression)               LOG_RAW("ERROR", expression)
   #define LOG_WARNING_RAW(expression)             LOG_RAW("WARNING", expression)
   #define LOG_INFORMATION_RAW(expression)         LOG_RAW("INFORMATION", expression)
   #define LOG_DEBUG_RAW(expression)               LOG_RAW("DEBUG", expression)
   #define LOG_VERBOSE_RAW(expression)             LOG_RAW("VERBOSE", expression)
   #define LOG_NONE_RAW(expression)                LOG_RAW("NONE", expression)

   #define LOG_FATAL_RAW2(tag, expression)         LOG_RAW2("FATAL", tag, expression)
   #define LOG_ERROR_RAW2(tag, expression)         LOG_RAW2("ERROR", tag, expression)
   #define LOG_WARNING_RAW2(tag, expression)       LOG_RAW2("WARNING", tag, expression)
   #define LOG_INFORMATION_RAW2(tag, expression)   LOG_RAW2("INFORMATION", tag, expression)
   #define LOG_DEBUG_RAW2(tag, expression)         LOG_RAW2("DEBUG", tag, expression)
   #define LOG_VERBOSE_RAW2(tag, expression)       LOG_RAW2("VERBOSE", tag, expression)
   #define LOG_NONE_RAW2(tag, expression)          LOG_RAW2("NONE", tag, expression)

   #define LOG_FATAL_RAW_IF(condition,expression)     LOG_RAW_IF("FATAL", condition, expression)
   #define LOG_ERROR_RAW_IF(condition,expression)     LOG_RAW_IF("ERROR", condition, expression)
   #define LOG_WARNING_RAW_IF(condition,expression)   LOG_RAW_IF("WARNING", condition, expression)
   #define LOG_INFORMATION_RAW_IF(condition,expression) LOG_RAW_IF("INFORMATION", condition, expression)
   #define LOG_DEBUG_RAW_IF(condition,expression)     LOG_RAW_IF("DEBUG", condition, expression)
   #define LOG_VERBOSE_RAW_IF(condition,expression)   LOG_RAW_IF("VERBOSE", condition, expression)
   #define LOG_NONE_RAW_IF(condition,expression)      LOG_RAW_IF("NONE", condition, expression)


   #define LOG_FATAL_IF(condition, expression)        LOG_IF("FATAL", condition, expression)
   #define LOG_ERROR_IF(condition, expression)        LOG_IF("ERROR", condition, expression)
   #define LOG_WARNING_IF(condition, expression)      LOG_IF("WARNING", condition, expression)
   #define LOG_INFORMATION_IF(condition, expression)  LOG_IF("INFORMATION", condition, expression)
   #define LOG_DEBUG_IF(condition, expression)        LOG_IF("DEBUG", condition, expression)
   #define LOG_VERBOSE_IF(condition, expression)      LOG_IF("VERBOSE", condition, expression)
   #define LOG_NONE_IF(condition, expression)         LOG_IF("NONE", condition, expression)


   #define LOG_FATAL_NR(uNumber, expression)       LOG_NR(uNumber, "FATAL", expression)
   #define LOG_ERROR_NR(uNumber, expression)       LOG_NR(uNumber, "ERROR", expression)
   #define LOG_WARNING_NR(uNumber, expression)     LOG_NR(uNumber, "WARNING", expression)
   #define LOG_INFORMATION_NR(uNumber, expression) LOG_NR(uNumber, "INFORMATION", expression)
   #define LOG_DEBUG_NR( uNumber, expression)      LOG_NR(uNumber, "DEBUG", expression)
   #define LOG_VERBOSE_NR(uNumber, expression)     LOG_NR(uNumber, "VERBOSE", expression)
   #define LOG_NONE_NR(uNumber, expression)        LOG_NR(uNumber, "NONE", expression)


   #define LOG_FATAL_IF_NR( uNumber, condition, expression)        LOG_IF_NR(uNumber, "FATAL", condition, expression)
   #define LOG_ERROR_IF_NR( uNumber, condition, expression)        LOG_IF_NR(uNumber, "ERROR", condition, expression)
   #define LOG_WARNING_IF_NR( uNumber, condition, expression)      LOG_IF_NR(uNumber, "WARNING", condition, expression)
   #define LOG_INFORMATION_IF_NR( uNumber, condition, expression)  LOG_IF_NR(uNumber, "INFORMATION", condition, expression)
   #define LOG_DEBUG_IF_NR( uNumber, condition, expression)        LOG_IF_NR(uNumber, "DEBUG", condition, expression)
   #define LOG_VERBOSE_IF_NR( uNumber, condition, expression)      LOG_IF_NR(uNumber, "VERBOSE", condition, expression)
   #define LOG_NONE_IF_NR( uNumber, condition, expression)         LOG_IF_NR(uNumber, "NONE", condition, expression)

#endif

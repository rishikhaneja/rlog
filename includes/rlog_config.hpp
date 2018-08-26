/**
 * @file rlog_config.hpp
 * @description configurables for rlog.hpp
 * @author Rishi Khaneja
 */

// -----------------------------------------------------------

#ifndef __R_LOG_CONFIG_HPP__
#define __R_LOG_CONFIG_HPP__

// -----------------------------------------------------------

/**
 * @brief Allows completely disabling RLog
 *        true: active, false: disabled
 */
#ifndef R_ACTIVE
#define R_ACTIVE (true)
#endif

// -----------------------------------------------------------

/**
 * @brief Allows filtering out all logging above specified level
 * @param Level
 */
#ifndef R_MIN_LEVEL
#define R_MIN_LEVEL (R::Level::Info)
#endif

// -----------------------------------------------------------

#endif  // __R_LOG_CONFIG_HPP__

// -----------------------------------------------------------
// EOF
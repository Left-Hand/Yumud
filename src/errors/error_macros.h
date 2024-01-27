#ifndef __ERROR_MACROS_H__

#define __ERROR_MACROS_H__

enum ErrorHandlerType {
	ERR_HANDLER_ERROR,
	ERR_HANDLER_WARNING,
	ERR_HANDLER_HARDWARE,
	// ERR_HANDLER_SHADER,
};

#define ERR_FAIL_COND_V_MSG(m_cond, m_retval, m_msg)                                                                                    \
	if (unlikely(m_cond)) {                                                                                                             \
		_err_print_error(FUNCTION_STR, __FILE__, __LINE__, "Condition \"" _STR(m_cond) "\" is true. Returned: " _STR(m_retval), m_msg); \
		return m_retval;                                                                                                                \
	} else                                                                                                                              \
		((void)0)

#define ERR_FAIL_V_MSG(m_retval, m_msg)                                                                          \
	if (true) {                                                                                                  \
		_err_print_error(FUNCTION_STR, __FILE__, __LINE__, "Method failed. Returning: " __STR(m_retval), m_msg); \
		return m_retval;                                                                                         \
	} else                                                                                                       \
		((void)0)

#define WARN_DEPRECATED_MSG(m_msg) \
	if (true) {  \
		static bool warning_shown;          \
		if (!warning_shown) {                                                                                                                            \
			_err_print_error(FUNCTION_STR, __FILE__, __LINE__, "This method has been deprecated and will be removed in the future.", m_msg, ERR_HANDLER_WARNING); \
            warning_shown = true; \
		}                         \
	} else \
		((void)0)

void _err_print_error(const char *p_function, const char *p_file, int p_line, const char *p_error, ErrorHandlerType p_type);

#endif
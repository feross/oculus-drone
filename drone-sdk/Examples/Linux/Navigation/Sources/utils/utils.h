#ifndef _UTILS_INCLUDE
#define _UTILS_INCLUDE

#include <VP_Os/vp_os_signal.h>

/************************************************************************
 * Defines
 ************************************************************************/
#define PRINTF_STRING_SIZE 256

/************************************************************************
 * Prototypes
 ************************************************************************/
void ThreadedPrintf(const char *format, ...);

void printBits(int val);

#include <VP_Os/vp_os_types.h>



///////////////////////////////////////////////
// DEFINES

#define RAD_TO_MDEG (57295.779513)
#define MDEG_TO_RAD (1.745329252e-05)


///////////////////////////////////////////////
// STRUCTURES

#define NB_FIRST_ORDER 	1
#define NB_SECOND_ORDER 2
#define NB_FOURTH_ORDER 4

/**
 * \struct _first_order_filter_
 * \brief  First order filter data (coefficients and history) using Matlab notation.
 */
typedef struct {
  float32_t a[NB_FIRST_ORDER+1];		/**< a coefficients (filter transfer function denominator coefficients) */
  float32_t b[NB_FIRST_ORDER+1];		/**< b coefficients (filter transfer function numerator coefficients) */
  float32_t old_outputs[NB_FIRST_ORDER];		/**< filter output history */
  float32_t old_inputs [NB_FIRST_ORDER];		/**< filter input history */
} first_order_filter_t;



/**
 * \struct _second_order_filter_
 * \brief  Second order filter data (coefficients and history) using Matlab notation.
 */
typedef struct {
  float32_t a[NB_SECOND_ORDER+1];		/**< a coefficients (filter transfer function denominator coefficients) */
  float32_t b[NB_SECOND_ORDER+1];		/**< b coefficients (filter transfer function numerator coefficients) */
  float32_t old_outputs[NB_SECOND_ORDER];		/**< filter output history */
  float32_t old_inputs [NB_SECOND_ORDER];		/**< filter input history */
} second_order_filter_t;

/**
 * \struct _fourth_order_filter_
 * \brief  Fourth order filter data (coefficients and history) using Matlab notation.
 */
typedef struct {
  float32_t a[NB_FOURTH_ORDER+1];		/**< a coefficients (filter transfer function denominator coefficients) */
  float32_t b[NB_FOURTH_ORDER+1];		/**< b coefficients (filter transfer function numerator coefficients) */
  float32_t old_outputs[NB_FOURTH_ORDER];		/**< filter output history */
  float32_t old_inputs [NB_FOURTH_ORDER];		/**< filter input history */
} fourth_order_filter_t;



///////////////////////////////////////////////
// FUNCTIONS

/**
 * \fn      Filter a value.
 * \brief   This function uses the same notation as Matlab except that array indices start at 0
 * \brief   a(1)*y(n) = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb) - a(2)*y(n-1) - ... - a(na+1)*y(n-na)
 * \brief   This function automatically shifts old inputs and outputs.
 * \param   Filter order
 * \param   address of B coefficients list.
  * \param   address of A coefficients list.
 * \param   input value.
 * \param   address of previous inputs list.
 * \param   address of previous outputs list.
 * \return  filtered value.
 *
 * \section History
 *
 * \par date: 2007-06-25  author: <florian.pantaleao.ext\@parrot.com> <jean-baptiste.lanfrey\@parrot.com>
 *  - first version
 */
float32_t filter(uint32_t n, float32_t *b, float32_t *a, float32_t input, float32_t *old_input, float32_t *old_output);

/**
 * \fn      Digital filter initialization.
 * \brief   This function is used to initialize previous values in digital filter.
 * \param   Filter order.
 * \param   address of previous inputs list.
 * \param   initial input value.
 * \param   address of previous outputs list.
 * \param   initial output value.
 * \return  void.
 *
 * \section History
 *
 * \par date: 2007-06-25  author: <florian.pantaleao.ext\@parrot.com> <jean-baptiste.lanfrey\@parrot.com>
 *  - first version
 */
void filter_init(uint32_t n, float32_t *old_input, float32_t initial_input,
		 float32_t *old_output, float32_t initial_output);


#endif

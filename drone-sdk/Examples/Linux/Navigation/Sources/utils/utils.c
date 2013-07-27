#include <stdio.h>
#include <stdarg.h>

#include <config.h>
#include <utils/utils.h>

vp_os_mutex_t PrintfLock;

/*
 * This function allows to print to the console from various threads without interferences
 */
void ThreadedPrintf(const char *format, ...)
{
	char tmpformat[PRINTF_STRING_SIZE];
	va_list varg;

	// Get the format of the command
	va_start(varg, format);
	vsprintf(tmpformat, format, varg);
	va_end(varg);

	vp_os_mutex_lock(&PrintfLock);

	printf("%s", tmpformat);
	fflush(stdout);

	vp_os_mutex_unlock(&PrintfLock);
}

void printBits(int val)
{
	int i;
	vp_os_mutex_lock(&PrintfLock);
	for(i = 0 ; i < 8 ; i++)
	{
		if (((val >> (7-i)) & 1))
			printf ("1");
		else
			printf ("0");
	}
	printf(" ");
	vp_os_mutex_unlock(&PrintfLock);
}


#ifndef EXTERNAL_CONTROL
float filter(uint32_t n, float *b, float *a, float input, float *old_input, float *old_output)
{
	uint32_t ii;
	float inno, past, output;
	
	// innovation computation
	inno = b[0] * input;
	for (ii=1; ii<n+1; ii++) {
		inno += b[ii] * old_input[ii-1];
	}
	
	// past computation
	past = 0.0;
	for (ii=1; ii<n+1; ii++) {
		past -= a[ii] * old_output[ii-1];
	}
	output = (inno + past) / a[0];

	// inputs and outputs shift
	for (ii=n-1; ii>0; ii--) {
		old_input[ii] = old_input[ii-1];
	}
	old_input[0] = input;
	
	for (ii=n-1; ii>0; ii--) {
		old_output[ii] = old_output[ii-1];
	}
	old_output[0] = output;

	return output;
}

void filter_init(uint32_t n, float *old_input, float initial_input,	float *old_output, float initial_output)
{
	int32_t ii;
	
	for (ii=0; ii<n ; ii++) {
		old_input[ii]  = initial_input;
	}
	for (ii=0; ii<n; ii++) {
		old_output[ii] = initial_output;
	}
}
#endif // ! EXTERNAL_CONTROL


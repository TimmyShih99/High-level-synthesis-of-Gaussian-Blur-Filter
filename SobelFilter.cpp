#include <cmath>
#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "SobelFilter.h"

SobelFilter::SobelFilter( sc_module_name n ): sc_module( n )
{
/*#ifndef NATIVE_SYSTEMC
	  HLS_FLATTEN_ARRAY(buffer);
#endif*/
	SC_THREAD( do_filter );
	sensitive << i_clk.pos();
	dont_initialize();
	reset_signal_is(i_rst, false);

#ifndef NATIVE_SYSTEMC
	i_rgb.clk_rst(i_clk, i_rst);
	o_result.clk_rst(i_clk, i_rst);
#endif
}

SobelFilter::~SobelFilter() {}

const int mask[5][5] = {{1, 4, 7, 4, 1}, {4, 16, 26, 16, 4}, {7, 26, 41, 26, 7}, {4, 16, 26, 16, 4}, {1, 4, 7, 4, 1}}; // unroll

void SobelFilter::do_filter() {
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		i_rgb.reset();
		o_result.reset();
#endif
		wait();
	}
  //int buffer[25];	
	uint8_t buffer[25]; // Improve coding style
#ifndef NATIVE_SYSTEMC
	  HLS_FLATTEN_ARRAY(buffer); // FLATTEN
#endif
	for (unsigned int u = 0 ; u<25; ++u) { // set buffer
	HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
		buffer[u] = 0;
	}

	while (true) {	
		HLS_CONSTRAIN_LATENCY( 0, HLS_ACHIEVABLE, "lat1" ); // reduce cycle counts
		HLS_PIPELINE_LOOP( HARD_STALL, 29, "main_loop_pipeline" );	// PIPELINE min 29

		for (unsigned int k = 24 ; k>=5; k-=1) { // store
			HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
			buffer[k] = buffer[k-5];
		}




		for (unsigned int v = 0; v<5; ++v) { // RGB
			HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
			sc_dt::sc_uint<8> Red, Green, Blue; 


#ifndef NATIVE_SYSTEMC
			{
				HLS_DEFINE_PROTOCOL("input");
				Red = i_rgb.get();
				wait();
				Green = i_rgb.get();
				wait();
				Blue = i_rgb.get();
				wait();
			}
#else
			Red = i_rgb.read();				
			Green = i_rgb.read();
			Blue = i_rgb.read();

#endif

			buffer[v] = (Red + Green + Blue)/3;
		} // RGB
		//auto val = 0;
		sc_dt::sc_uint<17> val = 0;
		for (unsigned int i = 0; i<5; ++i) { 
			HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
			for (unsigned int j = 0; j<5; ++j) { 
			HLS_UNROLL_LOOP(ON,"reads"); // UNROLL
				val += buffer[i*5+j]*mask[i][j];
			}
		}
		// Improve coding styles
		/*sc_dt::sc_uint<17> val =  
			buffer[0]*1   + 
			buffer[1]*4   + 
			buffer[2]*7   + 
			buffer[3]*4   + 	
			buffer[4]*1   +
			buffer[5]*4   + 
			buffer[6]*16  + 
			buffer[7]*26  + 
			buffer[8]*16  + 
			buffer[9]*4   + 
			buffer[10]*7  + 
			buffer[11]*26 + 
			buffer[12]*41 + 
			buffer[13]*26 + 
			buffer[14]*7  +
			buffer[15]*4  +
			buffer[16]*16 +
			buffer[17]*26 +
			buffer[18]*16 +
			buffer[19]*4  +
			buffer[20]*1  +
			buffer[21]*4  +
			buffer[22]*7  +
			buffer[23]*4  +
			buffer[24]*1  ;*/
		//sc_dt::sc_uint<8> total = val/273; // Improve coding styles
		//sc_dt::sc_uint<17> total = val;
#ifndef NATIVE_SYSTEMC
		{
			HLS_DEFINE_PROTOCOL("output");
			o_result.put(val);
			wait();
		}
#else
		o_result.write(val);
#endif



	} // While
} // do_filter()

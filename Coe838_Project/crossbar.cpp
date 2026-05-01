// crossbar.cpp
#include "packet.h"
#include "crossbar.h"

void crossbar :: func()
	{
	packet v_cross0;
	packet v_cross1;
	packet v_cross2;
	packet v_cross3;
	packet v_cross4;
	sc_uint<15>  v_config;

	// functionality
	while( true )
	{ 
		wait();
		v_config = config.read();
		if (i0.event())
		{  
			v_cross0 = i0.read();
			switch (v_config(2,0)) {
				// MODIFIED - AFTER(corrected):
				case 1: o0.write(v_cross0); break;  // local : sink
				case 2: o2.write(v_cross0); break;  // north : out2
				case 3: o3.write(v_cross0); break;  // east  : out3
				case 4: o4.write(v_cross0); break;  // south : out4
				case 5: o1.write(v_cross0); break;  // west  : out1
				default: cout << "---------------------------------wrong destination   " <<endl ;break ;
			}
		}
		if (i1.event())
		{
			v_cross1 = i1.read();
			switch (v_config(5,3)) {
				// MODIFIED - AFTER(corrected):
				case 1: o0.write(v_cross1); break;  // local : sink
				case 2: o2.write(v_cross1); break;  // north : out2
				case 3: o3.write(v_cross1); break;  // east  : out3
				case 4: o4.write(v_cross1); break;  // south : out4
				case 5: o1.write(v_cross1); break;  // west  : out1
				default: cout << "------------------------------------wrong destination   " <<endl;  break ;
			}
		}
		if (i2.event())
		{ 
			v_cross2 = i2.read();
			switch (v_config(8,6)) {
				// MODIFIED - AFTER(corrected):
				case 1: o0.write(v_cross2); break;  // local : sink
				case 2: o2.write(v_cross2); break;  // north : out2
				case 3: o3.write(v_cross2); break;  // east  : out3
				case 4: o4.write(v_cross2); break;  // south : out4
				case 5: o1.write(v_cross2); break;  // west  : out1
				default: cout << "---------------------------------wrong destination   " <<endl; break ;
			}
		}
		if (i3.event())
		{ 
			v_cross3 = i3.read();
			switch (v_config(11,9)) {
				// MODIFIED - AFTER(corrected):
				case 1: o0.write(v_cross3); break;  // local : sink
				case 2: o2.write(v_cross3); break;  // north : out2
				case 3: o3.write(v_cross3); break;  // east  : out3
				case 4: o4.write(v_cross3); break;  // south : out4
				case 5: o1.write(v_cross3); break;  // west  : out1
				default: cout << "----------------------------------wrong destination   " <<endl; break ;
			}
		}
		if (i4.event())
		{ 
			v_cross4 = i4.read();
			switch (v_config(14,12)) {
				// MODIFIED - AFTER(corrected):
				case 1: o0.write(v_cross4); break;  // local : sink
				case 2: o2.write(v_cross4); break;  // north : out2
				case 3: o3.write(v_cross4); break;  // east  : out3
				case 4: o4.write(v_cross4); break;  // south : out4
				case 5: o1.write(v_cross4); break;  // west  : out1
				default: cout << "-------------------------------------wrong destination" <<endl ;break ;
			}
		}
	}
}

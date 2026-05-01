// source.cpp
#include "source.h"
void source:: func()
	{
	packet v_packet_out;
	v_packet_out.data=1000; // e.g.
	v_packet_out.pkt_clk = '0';  // an imaginary clock for packets
	while(true)
		{
		wait();
		if(!ach_in.read())
			{
			v_packet_out.data = v_packet_out.data +source_id.read()+ 1  ; // made a desired data
			v_packet_out.id = source_id.read();
			v_packet_out.dest= (source_id.read() + 1) % 16;											// assign destination
			//if(v_packet_out.id == 1) goto exclode;	// prevent from reciving flits by itself 			 
			v_packet_out.pkt_clk= ~v_packet_out.pkt_clk ;  // add an imaginary clock to each flit 
			v_packet_out.h_t=false;
			pkt_snt++;
			v_packet_out.send_time = sc_time_stamp().to_double();//Record TIMESTAMP of source sending the packets
			
			if((pkt_snt%5)==0)v_packet_out.h_t=true; // make tail flit (the packet size is 5)
			packet_out.write(v_packet_out);

			cout << "New Pkt: " << v_packet_out.data << " is sent by source: " << source_id.read()  << "  to Destination:   "<< v_packet_out.dest <<endl;
exclode:;
			}  
		}
	}

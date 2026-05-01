// sink.cpp
#include "sink.h"
void sink::receive_data(){

	packet v_packet;
	if ( sclk.event() ) ack_out.write(false);
	if (packet_in.event() ) 
	{ 
		pkt_recv++ ;
		ack_out.write(true);
		v_packet= packet_in.read();

		double recv_time = sc_time_stamp().to_double();
		double delay = recv_time - v_packet.send_time;

		total_delay += delay;

		if(v_packet.h_t) { // only count packet at tail
			total_packets++;
			cout << "======== Average Delay for the packets(5 flits) so far: " << total_delay / total_packets << "========" << endl;
		    cout << "   ------------------------------------------------------------" << endl;
		}
		//cout << "			New Pkt:  " << (int)v_packet.data<< " is received from source: " << (int)v_packet.id  << " by sink:  " << (int)sink_id.read() << " With Delay:  " << (double)delay << endl;
		cout << "   >>> [SINK " << sink_id.read() << "] RECEIVED Packet from Source " << v_packet.id << " | Total Latency: " << (double)delay << "ns" << endl;
	}
}

// main.cpp (4x4 Mesh NoC)

#include "systemc.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "packet.h"
#include "source.h"
#include "sink.h"
#include "router.h"


int sc_main(int argc, char *argv[])
{
    // -------SIGNALS 
    sc_signal<packet> si_source[16];
    sc_signal<packet> si_sink[16];

    // Each directional link is a SEPARATE signal so each has exactly one driver.
    // Naming convention:  link index = row*3+col  for E-W  (3 per row, 4 rows = 12)
    //                     link index = row*4+col  for N-S  (4 per col-gap, 3 gaps = 12)
    sc_signal<packet> si_ew[12];       // East-bound  : router[i].out3 -> router[i+1].in1
    sc_signal<packet> si_we[12];       // West-bound  : router[i+1].out1 -> router[i].in3
    sc_signal<packet> si_ns[12];       // South-bound : router[i].out4 -> router[i+4].in2
    sc_signal<packet> si_sn[12];       // North-bound : router[i+4].out2 -> router[i].in4

    sc_signal<bool> si_ack_src[16],  si_ack_sink[16];
    sc_signal<bool> si_ack_ew[12],   si_ack_we[12];
    sc_signal<bool> si_ack_ns[12],   si_ack_sn[12];

    // dummy signals for unused boundary ports
    sc_signal<packet> si_zero[200];
    sc_signal<bool>   si_ack_zero[200];

    sc_signal<sc_uint<4> > id[16];

    sc_clock s_clock("S_CLOCK", 125, SC_NS, 0.5, 0.0, SC_NS); // source clock
    sc_clock r_clock("R_CLOCK", 5, SC_NS, 0.5, 10.0, SC_NS);	// router clock
    sc_clock d_clock("D_CLOCK", 5,   SC_NS, 0.5, 10.0, SC_NS);	// destination clock

    // Module instiatiations follow
    // Note that modules can be connected by hooking up ports
    // to signals by name or by using a positional notation
    source* src[16];
    sink*   snk[16];
    router* r[16];

    // -------INSTANTIATION 
    for(int i = 0; i < 16; i++)
    {
        src[i] = new source(sc_gen_unique_name("source"));
        src[i]->packet_out(si_source[i]);
        src[i]->source_id(id[i]);
        src[i]->ach_in(si_ack_src[i]);
        src[i]->CLK(s_clock);

        snk[i] = new sink(sc_gen_unique_name("sink"));
        snk[i]->packet_in(si_sink[i]);
        snk[i]->ack_out(si_ack_sink[i]);
        snk[i]->sink_id(id[i]);
        snk[i]->sclk(d_clock);

        r[i] = new router(sc_gen_unique_name("router"));
        r[i]->router_id(id[i]);
        r[i]->rclk(r_clock);

        // local
        r[i]->in0(si_source[i]);
        r[i]->out0(si_sink[i]);
        r[i]->outack0(si_ack_src[i]);
        r[i]->inack0(si_ack_sink[i]);
    }

    // ------CONNECTION 
    // hooking up signals to ports by name
    // Port mapping: in1/out1=West | in2/out2=North | in3/out3=East | in4/out4=South
    int z = 0;
    int za = 0;

    for(int i = 0; i < 16; i++)
    {
        int row = i / 4;
        int col = i % 4;

        // -------- WEST input  (receives from the eastbound signal of left neighbour) --------
        if(col > 0)
        {
            int link = row * 3 + (col - 1);   // E-W link index between col-1 and col
            r[i]->in1(si_ew[link]);
            r[i]->inack1(si_ack_ew[link]);
        }
        else
        {
            r[i]->in1(si_zero[z]);
            r[i]->inack1(si_ack_zero[za]);
            z++; za++;
        }

        // -------- NORTH input  (receives from the southbound signal of upper neighbour) --------
        if(row > 0)
        {
            int link = (row - 1) * 4 + col;   // N-S link index between row-1 and row
            r[i]->in2(si_ns[link]);
            r[i]->inack2(si_ack_ns[link]);
        }
        else
        {
            r[i]->in2(si_zero[z]);
            r[i]->inack2(si_ack_zero[za]);
            z++; za++;
        }

        // -------- EAST input  (receives from the westbound signal of right neighbour) --------
        if(col < 3)
        {
            int link = row * 3 + col;          // W-E link index between col and col+1
            r[i]->in3(si_we[link]);
            r[i]->inack3(si_ack_we[link]);
        }
        else
        {
            r[i]->in3(si_zero[z]);
            r[i]->inack3(si_ack_zero[za]);
            z++; za++;
        }

        // -------- SOUTH input  (receives from the northbound signal of lower neighbour) --------
        if(row < 3)
        {
            int link = row * 4 + col;          // S-N link index between row and row+1
            r[i]->in4(si_sn[link]);
            r[i]->inack4(si_ack_sn[link]);
        }
        else
        {
            r[i]->in4(si_zero[z]);
            r[i]->inack4(si_ack_zero[za]);
            z++; za++;
        }

        // ------------ OUTPUTS 

        // -------- WEST output  (drives the westbound signal toward left neighbour) --------
        if(col > 0)
        {
            int link = row * 3 + (col - 1);
            r[i]->out1(si_we[link]);           // this router is the sole driver of si_we[link]
            r[i]->outack1(si_ack_we[link]);
        }
        else
        {
            r[i]->out1(si_zero[z]);
            r[i]->outack1(si_ack_zero[za]);
            z++; za++;
        }

        // -------- NORTH output  (drives the northbound signal toward upper neighbour) --------
        if(row > 0)
        {
            int link = (row - 1) * 4 + col;
            r[i]->out2(si_sn[link]);           // this router is the sole driver of si_sn[link]
            r[i]->outack2(si_ack_sn[link]);
        }
        else
        {
            r[i]->out2(si_zero[z]);
            r[i]->outack2(si_ack_zero[za]);
            z++; za++;
        }

        // -------- EAST output  (drives the eastbound signal toward right neighbour) --------
        if(col < 3)
        {
            int link = row * 3 + col;
            r[i]->out3(si_ew[link]);           // this router is the sole driver of si_ew[link]
            r[i]->outack3(si_ack_ew[link]);
        }
        else
        {
            r[i]->out3(si_zero[z]);
            r[i]->outack3(si_ack_zero[za]);
            z++; za++;
        }

        // -------- SOUTH output  (drives the southbound signal toward lower neighbour) --------
        if(row < 3)
        {
            int link = row * 4 + col;
            r[i]->out4(si_ns[link]);           // this router is the sole driver of si_ns[link]
            r[i]->outack4(si_ack_ns[link]);
        }
        else
        {
            r[i]->out4(si_zero[z]);
            r[i]->outack4(si_ack_zero[za]);
            z++; za++;
        }
    }

//sc_start(0, SC_NS);
  // tracing:
    // trace file creation
    sc_trace_file *tf = sc_create_vcd_trace_file("graph");
    // External Signals
    sc_trace(tf, s_clock, "s_clock");
    sc_trace(tf, d_clock, "d_clock");
    sc_trace(tf, si_source[0],  "si_source[0]");
    sc_trace(tf, si_source[1],  "si_source[1]");
    sc_trace(tf, si_source[2],  "si_source[2]");
    sc_trace(tf, si_source[3],  "si_source[3]");
    sc_trace(tf, si_source[4],  "si_source[4]");
    sc_trace(tf, si_source[5],  "si_source[5]");
    sc_trace(tf, si_source[6],  "si_source[6]");
    sc_trace(tf, si_source[7],  "si_source[7]");
    sc_trace(tf, si_source[8],  "si_source[8]");
    sc_trace(tf, si_source[9],  "si_source[9]");
    sc_trace(tf, si_source[10], "si_source[10]");
    sc_trace(tf, si_source[11], "si_source[11]");
    sc_trace(tf, si_source[12], "si_source[12]");
    sc_trace(tf, si_source[13], "si_source[13]");
    sc_trace(tf, si_source[14], "si_source[14]");
    sc_trace(tf, si_source[15], "si_source[15]");
    sc_trace(tf, si_sink[0],    "si_sink[0]");
    sc_trace(tf, si_sink[1],    "si_sink[1]");
    sc_trace(tf, si_sink[2],    "si_sink[2]");
    sc_trace(tf, si_sink[3],    "si_sink[3]");
    sc_trace(tf, si_sink[4],    "si_sink[4]");
    sc_trace(tf, si_sink[5],    "si_sink[5]");
    sc_trace(tf, si_sink[6],    "si_sink[6]");
    sc_trace(tf, si_sink[7],    "si_sink[7]");
    sc_trace(tf, si_sink[8],    "si_sink[8]");
    sc_trace(tf, si_sink[9],    "si_sink[9]");
    sc_trace(tf, si_sink[10],   "si_sink[10]");
    sc_trace(tf, si_sink[11],   "si_sink[11]");
    sc_trace(tf, si_sink[12],   "si_sink[12]");
    sc_trace(tf, si_sink[13],   "si_sink[13]");
    sc_trace(tf, si_sink[14],   "si_sink[14]");
    sc_trace(tf, si_sink[15],   "si_sink[15]");

    for(int i = 0; i < 16; i++)
        id[i].write(i);

    cout << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << endl << " 4X4 mesh NOC simulator containing 16 5x5 Wormhole routers " << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "This is the simulation of a 4x4 Wormhole router.  " << endl;
    cout << "We assume the router has 5 input/output ports, with 4 buffers per input port " << endl;
    cout << "and each flit has 21 bits width " << endl;
    cout << "  Press \"Return\" key to start the simulation..." << endl << endl;

	cout << "\nCoordinate Mapping Legend:" << endl;
	for(int i = 0; i < 16; i++) {
		cout << "ID " << i << ":(" << i/4 << "," << i%4 << ")\t";
		if((i+1)%4 == 0) cout << endl;
	}
	cout << "\n--- STARTING TRAFFIC FLOW ---\n" << endl;

    getchar();
    sc_start(10*125+124, SC_NS); // during [(10*125)+124] ns 10 packets will be sent and received

    sc_close_vcd_trace_file(tf);

    cout << endl << endl << "-------------------------------------------------------------------------------" << endl;
    cout << "End of switch operation..." << endl;
    cout << "Total number of packets sent: " << endl;
    for(int i = 0; i < 16; i++)
        cout << "  source" << i << ": " << src[i]->pkt_snt << endl;
    cout << "Total number of packets received: " << endl;
    for(int i = 0; i < 16; i++)
        cout << "  sink" << i << ": " << snk[i]->pkt_recv << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "  Press \"Return\" key to end the simulation..." << endl << endl;
    getchar();
    return 0;

}



/*#include "systemc.h"
#include <iostream>
#include "packet.h"
#include "source.h"
#include "sink.h"
#include "router.h"

using namespace std;

int sc_main(int argc, char *argv[])
{
    // ================= SIGNALS =================
    sc_signal<packet> si_source[16];
    sc_signal<packet> si_sink[16];

    sc_signal<packet> si_h[24];   // horizontal links
    sc_signal<packet> si_v[24];   // vertical links

    sc_signal<bool> si_ack_src[16], si_ack_sink[16];
    sc_signal<bool> si_ack_h[24], si_ack_v[24];

    sc_signal<sc_uint<4>> id[16];

    // ================= CLOCKS =================
    sc_clock s_clock("S_CLOCK", 10, SC_NS);
    sc_clock r_clock("R_CLOCK", 10, SC_NS);
    sc_clock d_clock("D_CLOCK", 10, SC_NS);

    // ================= MODULES =================
    source* src[16];
    sink* snk[16];
    router* r[16];

    // ================= CREATE MODULES =================
    for(int i = 0; i < 16; i++)
    {
        src[i] = new source(sc_gen_unique_name("source"));
        src[i]->packet_out(si_source[i]);
        src[i]->source_id(id[i]);
        src[i]->ach_in(si_ack_src[i]);
        src[i]->CLK(s_clock);

        snk[i] = new sink(sc_gen_unique_name("sink"));
        snk[i]->packet_in(si_sink[i]);
        snk[i]->ack_out(si_ack_sink[i]);
        snk[i]->sink_id(id[i]);
        snk[i]->sclk(d_clock);

        r[i] = new router(sc_gen_unique_name("router"));
        r[i]->router_id(id[i]);
        r[i]->rclk(r_clock);

        // Local connection
        r[i]->in0(si_source[i]);
        r[i]->out0(si_sink[i]);

        r[i]->outack0(si_ack_src[i]);
        r[i]->inack0(si_ack_sink[i]);
    }

    // ================= CONNECT MESH =================
    int h = 0; // horizontal index
    int v = 0; // vertical index

    for(int i = 0; i < 16; i++)
    {
        int row = i / 4;
        int col = i % 4;

        // -------- EAST CONNECTION --------
        if(col < 3)
        {
            r[i]->out3(si_h[h]);      // current ? east
            r[i+1]->in1(si_h[h]);     // east router ? west input

            r[i]->outack3(si_ack_h[h]);
            r[i+1]->inack1(si_ack_h[h]);

            h++;
        }

        // -------- WEST CONNECTION --------
        if(col > 0)
        {
            r[i]->in1(si_h[h-1]);     // receive from west
            r[i-1]->out3(si_h[h-1]);  // west router sends east

            r[i]->inack1(si_ack_h[h-1]);
            r[i-1]->outack3(si_ack_h[h-1]);
        }

        // -------- SOUTH CONNECTION --------
        if(row < 3)
        {
            r[i]->out4(si_v[v]);      // current ? south
            r[i+4]->in2(si_v[v]);     // south router ? north input

            r[i]->outack4(si_ack_v[v]);
            r[i+4]->inack2(si_ack_v[v]);

            v++;
        }

        // -------- NORTH CONNECTION --------
        if(row > 0)
        {
            r[i]->in2(si_v[v-1]);     // receive from north
            r[i-4]->out4(si_v[v-1]);  // north router sends south

            r[i]->inack2(si_ack_v[v-1]);
            r[i-4]->outack4(si_ack_v[v-1]);
        }
    }

    // ================= ASSIGN IDS =================
    for(int i = 0; i < 16; i++)
        id[i].write(i);

    cout << "4x4 Mesh NoC Simulation Started..." << endl;

    sc_start(2000, SC_NS);

    cout << "Simulation Finished." << endl;

    return 0;
}*/
/*#include "systemc.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "packet.h"
#include "source.h"
#include "sink.h"
#include "router.h"


int sc_main(int argc, char *argv[])
{
	sc_signal<packet> si_source[4];
	sc_signal<packet> si_input[4];
	sc_signal<packet> si_zero[16];
	sc_signal<packet> si_sink[4];
	sc_signal<packet> si_output[4];
	sc_signal<bool>  si_ack_src[4],si_ack_ou[4];
	sc_signal<bool>  si_ack_sink[4],si_ack_in[4];
	sc_signal<bool>  si_ack_zero[16];
	sc_signal<sc_uint<4> >  id0,id1,id2,id3;
	sc_clock s_clock("S_CLOCK", 125, SC_NS, 0.5, 0.0, SC_NS); // source clock
	sc_clock r_clock("R_CLOCK", 125, SC_NS, 0.5, 0.0, SC_NS);	// router clock
	sc_clock d_clock("D_CLOCK", 5, SC_NS, 0.5, 10.0, SC_NS);	// destination clock
	
	// Module instiatiations follow
	// Note that modules can be connected by hooking up ports 
	// to signals by name or by using a positional notation
	source source0("source0");
	source0(si_source[0], id0, si_ack_src[0], s_clock);
	//need codes
	//need codes
	//need codes


	router router0("router0");
	// hooking up signals to ports by name
	router0.in0(si_source[0]);
	router0.in2(si_input[0]);
	router0.in3(si_input[3]);
	router0.in1(si_zero[0]);
	router0.in4(si_zero[1]);

	router0.router_id(id0);

	router0.out0(si_sink[0]);
	router0.out2(si_output[0]);
	router0.out3(si_output[3]);
	router0.out1(si_zero[2]);
	router0.out4(si_zero[3]);

	router0.inack0(si_ack_sink[0]);
	router0.inack2(si_ack_in[0]);
	router0.inack3(si_ack_in[3]);
	router0.inack1(si_ack_zero[0]);
	router0.inack4(si_ack_zero[1]);

	router0.outack0(si_ack_src[0]);
	router0.outack2(si_ack_ou[0]);
	router0.outack3(si_ack_ou[3]);
	router0.outack1(si_ack_zero[2]);
	router0.outack4(si_ack_zero[3]);

	router0.rclk(r_clock);

	router router1("router1");
	// hooking up signals to ports by name
	router1.in0(si_source[1]);
	router1.in2(si_zero[4]);
	router1.in3(si_input[1]);
	router1.in4(si_output[0]);
	router1.in1(si_zero[5]);

	router1.router_id(id1);

	router1.out0(si_sink[1]);
	router1.out2(si_zero[6]);
	router1.out3(si_output[1]);
	router1.out4(si_input[0]);
	router1.out1(si_zero[7]);

	router1.inack0(si_ack_sink[1]);
	router1.inack2(si_ack_zero[4]);
	router1.inack3(si_ack_in[1]);
	router1.inack4(si_ack_ou[0]);
	router1.inack1(si_ack_zero[5]);

	router1.outack0(si_ack_src[1]);
	router1.outack2(si_ack_zero[6]);
	router1.outack3(si_ack_ou[1]);
	router1.outack4(si_ack_in[0]);
	router1.outack1(si_ack_zero[7]);

	router1.rclk(r_clock);
	//need 64 code statement


  	//need codes
	sink sink1("sink1");
	sink1(si_sink[1], si_ack_sink[1], id1, d_clock);
	//need codes
	//need codes

//sc_start(0, SC_NS);
  // tracing:
	// trace file creation
	sc_trace_file *tf = sc_create_vcd_trace_file("graph");
	// External Signals
	sc_trace(tf, s_clock, "s_clock");
	sc_trace(tf, d_clock, "d_clock");
	sc_trace(tf, si_source[0], "si_source[0]");
	sc_trace(tf, si_source[0], "si_source[1]");
	//need codes
	//need codes
	sc_trace(tf, si_source[0], "si_sink[0]");
	sc_trace(tf, si_source[0], "si_sink[1]");
	//need codes
	//need codes
	
	id0.write(0);
	id1.write(1);
	//need codes
	//need codes

	cout << endl;
	cout << "-------------------------------------------------------------------------------" << endl;
	cout << endl << " 1X2 mesh NOC simulator containing 2 5x5 Wormhole router " << endl;
	cout << "-------------------------------------------------------------------------------" << endl;
	cout << "This is the simulation of a 1x2 Wormhole router.  " << endl; 
	cout << "We assume the router has 5 input/output ports, with 4 buffers per input port " << endl;
	cout << "and each flit has 21 bits width " << endl;
	cout << "  Press \"Return\" key to start the simulation..." << endl << endl;

	getchar();
	sc_start(10*125+124, SC_NS); // during [(10*125)+124] ns 10 packets will be sent and received 

	sc_close_vcd_trace_file(tf);

	cout << endl << endl << "-------------------------------------------------------------------------------" << endl;
	cout << "End of switch operation..." << endl;
	cout << "Total number of packets sent: " <<  source0.pkt_snt<< endl;//need codes to be added
	cout << "Total number of packets received: " <<  sink1.pkt_recv<< endl;//need codes to be added
	cout << "-------------------------------------------------------------------------------" << endl;
    cout << "  Press \"Return\" key to end the simulation..." << endl << endl;
	getchar();
  return 0;

}*/


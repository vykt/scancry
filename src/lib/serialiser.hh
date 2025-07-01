#pragma once

//local headers
#include "scancry.h"



extern "C" {

//sc_serialiser

sc_serialiser sc_new_serialiser();
int sc_del_serialiser(sc_serialiser serialiser);
int sc_save_scan(sc_serialiser serialiser,
                 sc_scan scan, const sc_opt opts);
int sc_load_scan(sc_serialiser serialiser,
                 sc_scan scan, const sc_opt opts, const bool shallow);
int sc_read_headers(sc_serialiser serialiser,
                    const char * file_path, sc_combined_file_hdr * cmb_hdr);

}

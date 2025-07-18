//standard template library
#include <iostream>
#include <iomanip>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <doctest/doctest.h>

//local headers
#include "common.hh"
#include "memcry_helper.hh"




void _memcry_helper::setup(_memcry_helper::args & mcry_args, pid_t pid, int session_num) {

    int ret;


    //start MemCry sessions on the target
    mcry_args.sessions.resize(session_num);
    for (int i = 0; i < session_num; ++i) {
        ret = mc_open(&mcry_args.sessions[i], PROCFS, pid);
        CHECK_EQ(ret, 0);
    }

    //initialise a MemCry map of the target
    mc_new_vm_map(&mcry_args.map);

    //populate the MemCry map
    ret = mc_update_map(&mcry_args.sessions[0], &mcry_args.map);
    REQUIRE_EQ(ret, 0);

    return;
}


void _memcry_helper::teardown(_memcry_helper::args & mcry_args) {

    int ret;
    

    //close MemCry sessions
    for (auto iter = mcry_args.sessions.begin();
         iter != mcry_args.sessions.end(); ++iter) {
        ret = mc_close(&(*iter));
        CHECK_EQ(ret, 0);
    }
    mcry_args.sessions.resize(0);
    
    //destroy the MemCry map
    ret = mc_del_vm_map(&mcry_args.map);
    CHECK_EQ(ret ,0);

    return;
}


void _memcry_helper::print_area(mc_vm_area * area) {

    char str_buf[5];


    //convert access to a string
    mc_access_to_str(area->access, str_buf);

    /*
     *  NOTE: Format: <start_addr> - <end_addr> - <perms> - <basename:12>
     */
     
    std::cout << std::hex;
    std::cout << "0x" << area->start_addr << " - 0x" << area->end_addr;
    std::cout << " | " << std::string(str_buf) << " | ";
    if (area->basename != nullptr) {
        std::cout << std::left << std::setw(10);
        std::cout << std::string(area->basename).substr(0, 12);
    }
    std::cout << std::endl; 
}


void _memcry_helper::print_map(mc_vm_map * map) {

    char str_buf[5];
    cm_lst_node * area_node;
    mc_vm_area * area;


    //for every area
    area_node = map->vm_areas.head;
    for (int i = 0; i < map->vm_areas.len; ++i) {

        //fetch area
        area = MC_GET_NODE_AREA(area_node);

        //dump area
        _memcry_helper::print_area(area);        

        //fetch next area
        area_node = area_node->next;
    }
    
}

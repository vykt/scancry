//standard template library
#include <iostream>
#include <iomanip>

//C standard library
#include <cstring>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <doctest/doctest.h>

//local headers
#include "common.hh"
#include "memcry_helper.hh"




void _memcry_helper::setup(_memcry_helper::args & mcry_args,
                           const pid_t pid, const int session_num) {

    int ret;
    mc_session * session;


    //start memcry sessions on the target
    ret = cm_new_vct(&mcry_args.sessions, sizeof(mc_session));
    REQUIRE_EQ(ret, 0);

    ret = cm_vct_rsz(&mcry_args.sessions, session_num);
    REQUIRE_EQ(ret, 0);
    
    for (int i = 0; i < mcry_args.sessions.len; ++i) {

        session = (mc_session *) cm_vct_get_p(&mcry_args.sessions, i);
        REQUIRE_NE(session, nullptr);
        
        std::memset(session, 0, sizeof(*session));
        ret = mc_open(session, PROCFS, pid);
        REQUIRE_EQ(ret, 0);
    }

    //initialise a memcry map of the target
    mc_new_vm_map(&mcry_args.map);

    //populate the memcry map
    ret = mc_update_map(session, &mcry_args.map);
    REQUIRE_EQ(ret, 0);

    return;
}


void _memcry_helper::teardown(_memcry_helper::args & mcry_args) {

    int ret;
    mc_session * session;


    //close MemCry sessions
    for (int i = 0;  i < mcry_args.sessions.len; ++i) {

        session = (mc_session *) cm_vct_get_p(&mcry_args.sessions, i);
        REQUIRE_NE(session, nullptr);

        ret = mc_close(session);
        REQUIRE_EQ(ret, 0);
    }

    //destroy the sessions vector
    cm_del_vct(&mcry_args.sessions);
    
    //destroy the memory map
    ret = mc_del_vm_map(&mcry_args.map);
    REQUIRE_EQ(ret ,0);

    return;
}


void _memcry_helper::print_area(const mc_vm_area * area) {

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


void _memcry_helper::print_map(const mc_vm_map * map) {

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

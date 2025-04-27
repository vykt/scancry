//standard template library
#include <optional>
#include <memory>
#include <vector>
#include <list>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <functional>
#include <exception>

//C standard library
#include <cstring>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>

//local headers
#include "scancry.h"
#include "ptrscan.hh"
#include "fbuf_util.hh"
#include "error.hh"



/*
 *  --- [TREE NODE | PUBLIC] ---
 */

//connect a node as a child of another node
void sc::_ptrscan_tree_node::connect_child(
    const std::shared_ptr<_ptrscan_tree_node> child_node) {

    this->children.push_back(child_node);
    return;
}


void sc::_ptrscan_tree_node::clear() {
    this->children.clear();
}


//get a reference to the children of a node
[[nodiscard]] const std::list<std::shared_ptr<sc::_ptrscan_tree_node>>
    & sc::_ptrscan_tree_node::get_children() const noexcept {

    return this->children;
}


[[nodiscard]] bool sc::_ptrscan_tree_node::has_children() const noexcept {
    return this->children.empty();
}



/*
 *  --- [TREE | PRIVATE] ---
 */

void sc::_ptrscan_tree::add_node(std::shared_ptr<sc::_ptrscan_tree_node> node,
                                 const cm_lst_node * area_node,
                                 const int depth_level,
                                 const uintptr_t own_addr,
                                 const uintptr_t ptr_addr) {

    //create new node
    std::shared_ptr<sc::_ptrscan_tree_node> new_node
        = std::make_shared<sc::_ptrscan_tree_node>(this->next_id, area_node,
                                                   own_addr, ptr_addr, node);

    //add new node to its parent
    node->connect_child(new_node);

    //add new node to its depth level list
    this->depth_levels[depth_level].push_back(new_node);

    return;
}


void sc::_ptrscan_tree::reset() {

    //free tree node depth layers in bottom-up order
    for (auto iter = this->depth_levels.rbegin();
         iter != this->depth_levels.rend(); ++iter) {

        iter->clear();
    }

    //reset variables
    this->depth_levels.clear();
    this->root_node.reset();
    this->next_id = 0;

    return;
}


[[nodiscard]] pthread_mutex_t & sc::_ptrscan_tree::get_write_mutex() noexcept {
    return this->write_mutex;
}


[[nodiscard]] const std::vector<std::shared_ptr<sc::_ptrscan_tree_node>> &
    sc::_ptrscan_tree::get_depth_level_vct(int level) const noexcept {

    return this->depth_levels[level];
}


[[nodiscard]] std::vector<
    std::vector<std::shared_ptr<sc::_ptrscan_tree_node>>>
        ::const_reverse_iterator
            sc::_ptrscan_tree::get_depth_level_crbegin() const noexcept {

    return this->depth_levels.crbegin();
}


[[nodiscard]] std::vector<
    std::vector<std::shared_ptr<sc::_ptrscan_tree_node>>>
        ::const_reverse_iterator
            sc::_ptrscan_tree::get_depth_level_crend() const noexcept {

    return this->depth_levels.crend();
}


[[nodiscard]] const std::shared_ptr<sc::_ptrscan_tree_node>
    sc::_ptrscan_tree::get_root_node() const {

    return this->root_node;
}



/*
 *  --- [POINTER SCANNER CHAIN | PUBLIC] ---
 */

sc::ptrscan_chain::ptrscan_chain(const cm_lst_node * obj_node,
                                 const uint32_t _obj_idx,
                                 const std::vector<off_t> & offsets)
                                  : obj_node((cm_lst_node *) obj_node),
                                    _obj_idx(_obj_idx),
                                    offsets(offsets) {}

sc::ptrscan_chain::ptrscan_chain(const std::string pathname,
                                 const uint32_t _obj_idx,
                                 const std::vector<off_t> & offsets)
                                  : pathname(pathname),
                                    _obj_idx(_obj_idx),
                                    offsets(offsets) {}


/*
 *  --- [POINTER SCANNER | PRIVATE] ---
 */

void sc::ptrscan::add_node(std::shared_ptr<sc::_ptrscan_tree_node> parent_node,
                           const cm_lst_node * area_node,
                           const uintptr_t own_addr,
                           const uintptr_t ptr_addr) {

    //create node inside the ptrscan tree
    this->tree_p->add_node(
        parent_node, area_node, this->cur_depth_level, own_addr, ptr_addr);

    return;
}


[[nodiscard]] std::pair<std::string, cm_lst_node *>
    sc::ptrscan::get_chain_data(
        const cm_lst_node * const area_node) const {

    mc_vm_area * area;

    cm_lst_node * obj_node;
    mc_vm_obj * obj;


    //fetch the appropriate object
    area = MC_GET_NODE_AREA(area_node);
    obj_node = (area->obj_node_p == nullptr)
               ? area->last_obj_node_p : area->obj_node_p;
    obj = MC_GET_NODE_OBJ(area->last_obj_node_p);
        
    return std::pair<std::string, cm_lst_node *>(obj->pathname, obj_node);
}


[[nodiscard]] _SC_DBG_INLINE int
    sc::ptrscan::get_chain_idx(const std::string & pathname) {

    //find if pathname is already present in the serialised pathnames vector
    auto iter = std::find(this->ser_pathnames.begin(),
                          ser_pathnames.end(), pathname);

    //if not present, add it and return its index
    if (iter == ser_pathnames.end()) {
        this->ser_pathnames.push_back(pathname);
        return this->ser_pathnames.size() - 1;
    }

    //else return index
    return std::distance(this->ser_pathnames.begin(), iter);
}


[[nodiscard]] std::pair<size_t, size_t>
    sc::ptrscan::get_fbuf_data_sz() const {

    size_t pathnames_sz = 0, chains_sz = 0;

    //add-up pathname sizes
    for (auto iter = this->ser_pathnames.begin();
         iter != this->ser_pathnames.end(); ++iter) {

        //pathname string + null terminator
        pathnames_sz += iter->size() + 1;
    }

    //additional null terminator to denote the end of pathnames
    pathnames_sz += 1;

    //add-up chain sizes
    for (auto iter = this->chains.begin();
         iter != this->chains.end(); ++iter) {

        //pathname index
        chains_sz += 4;

        //offsets & continue/end bytes
        chains_sz += iter->offsets.size() * 5;
    }

    //end of each chain contains a continue/end byte
    chains_sz += this->chains.size();

    return std::pair<size_t, size_t>(pathnames_sz, chains_sz);
}


//interpret the start of the file buffer
[[nodiscard]] int sc::ptrscan::handle_body_start(
    const std::vector<cm_byte> & buf, off_t hdr_off, off_t & buf_off) {

    //fetch the ptrscan header
    std::optional<struct _ptrscan_file_hdr> local_hdr
        = fbuf_util::unpack_type<struct _ptrscan_file_hdr>(buf, buf_off);
    if (local_hdr.has_value() == false) return -1;

    //fetch every pathname
    do {

        //fetch the pathname string
        std::optional<std::string> pathname
            = fbuf_util::unpack_string(buf, buf_off);
        if (pathname.has_value() == false) return -1;

        //if there are no more strings, stop
        if (pathname->empty()) break;

        //otherwise add this pathname to the pathnames vector
        this->ser_pathnames.push_back(pathname.value());

    } while(true);

    return 0;
}


//interpret a single chain in the file buffer
[[nodiscard]] std::optional<std::pair<uint32_t, std::vector<off_t>>>
    sc::ptrscan::handle_body_chain(
        const std::vector<cm_byte> & buf, off_t & buf_off) {

    std::vector<off_t> offsets;

    
    //fetch the object index
    std::optional<uint32_t> obj_idx
        = fbuf_util::unpack_type<uint32_t>(buf, buf_off);
    if (obj_idx.has_value() == false) return std::nullopt;

    //fetch the chain array
    std::optional<std::vector<uint32_t>> chain_arr
        = fbuf_util::unpack_type_array<uint32_t>(buf, buf_off);        
    if (obj_idx.has_value() == false) return std::nullopt;

    //convert on-disk `uint32_t` to `off_t`
    for (auto iter = chain_arr->begin();
         iter != chain_arr->end(); ++ iter) {
        offsets.push_back((off_t) *iter);
    }

    return std::pair<uint32_t, std::vector<off_t>>(*obj_idx, offsets);
}


[[nodiscard]] int sc::ptrscan::flatten_tree() {

    /*
     *  NOTE: To extract individual pointer chains from the pointer scan
     *        tree, each leaf node is followed up until it reaches the
     *        root node. Only the starting area is recorded.
     *
     */

    int idx;
    off_t offset;
    std::shared_ptr<sc::_ptrscan_tree_node> child, parent;


    //for every depth level
    for (auto iter = ++this->tree_p->get_depth_level_crbegin();
         iter != --this->tree_p->get_depth_level_crend(); ++iter) {

        //for every node at this depth level
        for (auto inner_iter = iter->cbegin();
             inner_iter != iter->cend(); ++inner_iter) {

            //fetch node
            const std::shared_ptr<sc::_ptrscan_tree_node> node = *inner_iter;

            //skip this node if it is not a leaf node
            if (node->has_children() == true) continue;

            /* While it is tempting to recurse here, it is slow. */

            //for each tree edge from this leaf to the root, add a chain entry
            std::vector<off_t> offsets;
            child = node;
            do {
                //fetch parent
                parent = node->parent;

                //add offset
                offset = parent->own_addr - child->ptr_addr;
                offsets.push_back(offset);

            } while (parent != this->tree_p->get_root_node());

            //fetch data for this chain
            auto chain_data = this->get_chain_data(node->area_node);
            idx = this->get_chain_idx(chain_data.first);

            //add chain
            this->chains.emplace_back(ptrscan_chain(chain_data.second,
                                                    idx, offsets));

        } //end for every node at this depth level

    } //end for every depth level

    return 0;
}


/*
 *  NOTE: To verify if a chain is valid, simply attempt to follow it.
 *        If it arrives at the expected address, it is valid. Failure
 *        can be caused by both incorrect final address and failure to
 *        read (e.g.: trying to read unmapped memory).
 */

[[nodiscard]] bool is_chain_valid(const uintptr_t target_addr,
                                  const struct sc::ptrscan_chain & chain,
                                  mc_session & session) {

    int ret;
    uintptr_t addr;
    
    cm_lst_node * obj_node;
    mc_vm_obj * obj;


    //return false if chain is malformed
    if ((chain.obj_node.has_value() == false)
        || (chain.obj_node.value() == nullptr)) return false;

    //get relevant object & bootstrap iteration
    obj_node = chain.obj_node.value();
    obj = MC_GET_NODE_OBJ(obj_node);
    addr = obj->start_addr;

    //for every offset
    for (auto iter = chain.offsets.begin();
         iter != chain.offsets.end(); ++iter) {

        //read next address
        addr += *iter;
        ret = mc_read(&session, addr, (cm_byte *) &addr, sizeof(addr));
        if (ret != 0) return false;
    }

    return (addr == target_addr) ? true : false;
}


void sc::ptrscan::do_reset() {
    
    //reset variables
    this->tree_p->reset();
    this->cur_depth_level = 0;
    
    this->ser_pathnames.clear();
    this->ser_pathnames.shrink_to_fit();

    this->chains.clear();
    this->chains.shrink_to_fit();

    //reset cache
    this->cache.serial_buf.clear();
    this->cache.serial_buf.shrink_to_fit();

    return;
}



/*
 *  --- [POINTER SCANNER | INTERFACE] ---
 */

//temporary storage for a potential new node in the ptrscan tree
struct _potential_node {

    //[attributes]
    const uintptr_t own_addr;
    const uintptr_t ptr_addr;
    const std::shared_ptr<sc::_ptrscan_tree_node> parent_tree_node;
    const cm_lst_node * area_node;

    //[methods]
    _potential_node(
            const uintptr_t own_addr,
            const uintptr_t ptr_addr,
            const std::shared_ptr<sc::_ptrscan_tree_node> parent_tree_node,
            const cm_lst_node * area_node)
     : own_addr(own_addr), ptr_addr(ptr_addr),
       parent_tree_node(parent_tree_node), area_node(area_node) {}
};


//process a single address from a worker thread
[[nodiscard]] int sc::ptrscan::_process_addr(
                                    const struct _scan_arg arg,
                                    const opt * const opts,
                                    const _opt_scan * const opts_scan) {

    /*
     *  NOTE: This function is called for each byte of memory that is
     *        scanned; it is imperative that the most common fail cases
     *        are considered first.
     */

    /*
     *  NOTE: `_manage_scan()` already verified the type of `opts_scan`.
     *        It's safe to cast directly.
     */

    //fetch ptrscan options
    const opt_ptrscan * const opts_ptrscan
        = (const opt_ptrscan * const) opts_scan;

    //if not on an alignment boundary, return
    if ((arg.area_off % opts_ptrscan->get_alignment().value()) != 0) return 0;

    //if not enough space is left in the buffer to hold a pointer
    /* FIXME [should be impossible, remove?]
    off_t required_left = (opts.addr_width == sc::AW64) ? 8 : 4;
    if (arg.buf_left < required_left) return 0;
    */

    /*
     *  `const` qualifier is discarded; no better way to do this the way
     *  things are currently organised.
     */

    //re-cache the depth level vector at the start of every area
    if (arg.area_off == 0)
        this->cache.depth_level_vct =
            (std::vector<std::shared_ptr<sc::_ptrscan_tree_node>> *)
            &this->tree_p->get_depth_level_vct(this->cur_depth_level);

    /*
     *  NOTE: `new_nodes` stores nodes that will be added by the end of
     *        this call (if any). In case a smart scan is being performed,
     *        this vector will be reset each time a new minimum offset is
     *        found.
     *
     *  TODO: Determine whether more than a single minimum is possible
     *        during a smart scan. For now assume that it is.
     */

    //setup new node container
    std::vector<struct _potential_node> new_nodes;
    off_t min_obj_sz = opts_ptrscan->get_max_obj_sz().value();

    //get potential pointer value
    uintptr_t potential_ptr;
    if (opts->addr_width == sc::AW32)
        potential_ptr = *((uint32_t *) arg.cur_byte);
    if (opts->addr_width == sc::AW64)
        potential_ptr = *((uint64_t *) arg.cur_byte);


    /*
     *  NOTE: Iteration over each potential parent node begins now.
     */

    //for every ptrscan tree node at this depth
    for (auto level_iter = this->cache.depth_level_vct->begin();
         level_iter != this->cache.depth_level_vct->end(); ++level_iter) {

        //get the current node from the iterator
        const std::shared_ptr<sc::_ptrscan_tree_node> & now_node
            = *level_iter;

        //if this potential pointer falls outside the range of this
        //node, skip it
        if (potential_ptr <
                (now_node->own_addr - opts_ptrscan->get_max_obj_sz().value())
            || potential_ptr > (now_node->own_addr)) continue;


        //else this is a match

        //check the offset is correct, if one applies
        auto presets = opts_ptrscan->get_preset_offsets();
        if (presets.has_value() && presets->size() <= this->cur_depth_level)
            if (potential_ptr != (now_node->own_addr
                    - (*presets)[this->cur_depth_level])) continue;
        
        //if this is a smart scan, manipulate the new node container
        if (opts_ptrscan->get_smart_scan() == true) {

            //if greater than current minimum, ignore this match
            if ((now_node->own_addr - potential_ptr)
                 > min_obj_sz) continue;

            //if smaller than current minimum, reset the node container
            if ((now_node->own_addr - potential_ptr)
                 < min_obj_sz) new_nodes.clear();

        }

        //add this node to the new node container
        new_nodes.push_back(_potential_node(arg.addr, potential_ptr,
                                            now_node, arg.area_node));
        
    } //end for every ptrscan tree node at this depth


    /*
     *  NOTE: Adding nodes all at once at tne end also minimises
     *        mutex thrashing.
     */

    //acquire the mutex
    int ret = pthread_mutex_lock(&this->tree_p->get_write_mutex());
    if (ret != 0) {
        sc_errno = SC_ERR_PTHREAD;
        return -1;
    }

    //add every new node to the tree
    for (auto new_iter = new_nodes.begin();
         new_iter != new_nodes.end(); ++new_iter) {

        //add the new node to the tree
        this->tree_p->add_node(new_iter->parent_tree_node,
                               new_iter->area_node,
                               this->cur_depth_level,
                               new_iter->own_addr,
                               new_iter->ptr_addr);
    }

    //release the mutex
    ret = pthread_mutex_unlock(&this->tree_p->get_write_mutex());
    if (ret != 0) {
        sc_errno = SC_ERR_PTHREAD;
        return -1;
    }

    return 0;
}


[[nodiscard]] int sc::ptrscan::_generate_body(
    std::vector<cm_byte> & buf, const off_t hdr_off) {

    std::optional<int> ret;
    struct _ptrscan_file_hdr local_hdr;

    cm_byte ctrl_byte;
    uint32_t off32;

    off_t buf_off = 0;
    std::pair<size_t, size_t> ptrscan_data_szs;


    //lock scanner
    _LOCK(-1)

    //check the scan contains a result to serialise
    if (this->chains.empty() == true) {
        sc_errno = SC_ERR_NO_RESULT;
        goto _read_body_fail;
    }

    //get the size of pathnames & chains
    ptrscan_data_szs = this->get_fbuf_data_sz();

    //build local header
    local_hdr.pathnames_num = this->ser_pathnames.size();
    local_hdr.pathnames_offset = hdr_off + sizeof(local_hdr);
    local_hdr.chains_num = this->chains.size();
    local_hdr.chains_offset = hdr_off
                              + sizeof(local_hdr)
                              + ptrscan_data_szs.first;

    //allocate space in the vector for the data
    buf.resize(sizeof(local_hdr)
               + ptrscan_data_szs.first + ptrscan_data_szs.second + 1);


    //store the header
    ret = fbuf_util::pack_type<struct _ptrscan_file_hdr>(
                                                buf, buf_off, local_hdr);
    if (ret.has_value() == false) goto _read_body_fail;
    

    //store every pathname
    for (auto iter = this->ser_pathnames.begin();
         iter != this->ser_pathnames.end(); ++iter) {

        ret = fbuf_util::pack_string(buf, buf_off, *iter);
        if (ret.has_value() == false) goto _read_body_fail;
    }

    //store an additional null terminator to denote the end of pathnames
    ctrl_byte = 0x00;
    ret = fbuf_util::pack_type(buf, buf_off, ctrl_byte);
    if (ret.has_value() == false) goto _read_body_fail;
    

    //store every chain
    for (auto iter = this->chains.begin();
         iter != this->chains.end(); ++iter) {

        //store pathname index
        ret = fbuf_util::pack_type(buf, buf_off, iter->_obj_idx);
        if (ret.has_value() == false) goto _read_body_fail;

        //store every offset
        ret = fbuf_util::pack_type_array(buf, buf_off, iter->offsets);
        if (ret.has_value() == false) goto _read_body_fail;
    }

    //store the file end byte
    ctrl_byte = fbuf_util::_file_end;
    ret = fbuf_util::pack_type(buf, buf_off, ctrl_byte);
    if (ret.has_value() == false) goto _read_body_fail;

    _UNLOCK(-1)
    return 0;

    _read_body_fail:
    _UNLOCK(-1)
    return -1;
}


[[nodiscard]] int sc::ptrscan::_process_body(
    const std::vector<cm_byte> & buf, off_t hdr_off, const mc_vm_map & map) {

    std::optional<int> ret;
    std::optional<std::pair<uint32_t, std::vector<off_t>>> inprog_chain;

    off_t buf_off = 0;

    cm_lst_node * obj_node;
    std::unordered_map<std::string, cm_lst_node *> obj_node_map;


    //lock scanner
    _LOCK(-1)

    //process the start of the header
    ret = this->handle_body_start(buf, hdr_off, buf_off);
    if (ret.has_value() == false) goto _process_body_fail;

    //associate each read pathname to a vm_obj node if one is present
    for (auto iter = this->ser_pathnames.begin();
         iter != this->ser_pathnames.end(); ++iter) {

        obj_node = mc_get_obj_by_pathname(&map, iter->c_str());        
        obj_node_map[*iter] = obj_node;
    }


    //fetch each chain
    do {

        //get pathname index & offset chains
        inprog_chain = this->handle_body_chain(buf, buf_off);
        if (inprog_chain.has_value() == false) {
            this->ser_pathnames.clear();
            this->ser_pathnames.shrink_to_fit();
            goto _process_body_fail;
        }

        //fetch the MemCry object for this pathname, if one is present
        obj_node = mc_get_obj_by_pathname(
                      &map, this->ser_pathnames[inprog_chain->first].c_str());
        if (obj_node == nullptr) continue;

        //add this chain to the chain list
        this->chains.emplace_back(ptrscan_chain(
            obj_node, inprog_chain->first, inprog_chain->second));

        if ((buf.size() >= buf_off) || buf[buf_off] == fbuf_util::_file_end)
            break;

    } while (true);

    _UNLOCK(-1)
    return 0;

    _process_body_fail:
    _UNLOCK(-1)
    return -1;
}


[[nodiscard]] int sc::ptrscan::_read_body(
    const std::vector<cm_byte> & buf, off_t hdr_off) {

    std::optional<int> ret;
    std::optional<std::pair<uint32_t, std::vector<off_t>>> inprog_chain;

    off_t buf_off = 0;


    //lock scanner
    _LOCK(-1)

    //process the start of the header
    ret = this->handle_body_start(buf, hdr_off, buf_off);
    if (ret.has_value() == false) goto _read_body_fail;


    //fetch each chain
    do {

        //get pathname index & offset chains
        inprog_chain = this->handle_body_chain(buf, buf_off);
        if (inprog_chain.has_value() == false) {
            this->ser_pathnames.clear();
            this->ser_pathnames.shrink_to_fit();
            goto _read_body_fail;
        }

        //add this chain to the chain list
        this->chains.emplace_back(ptrscan_chain(
            this->ser_pathnames[inprog_chain->first],
            inprog_chain->first, inprog_chain->second));

        if ((buf.size() >= buf_off) || buf[buf_off] == fbuf_util::_file_end)
            break;

    } while (true);
    
    _UNLOCK(-1)
    return 0;

    _read_body_fail:
    _UNLOCK(-1)
    return -1;
}



/*
 *  --- [POINTER SCANNER | PUBLIC] ---
 */

sc::ptrscan::ptrscan()
 : _scan(),
   cur_depth_level(0),
   cache({0}) {}


[[nodiscard]] int sc::ptrscan::reset() {

    _LOCK(-1);
    this->do_reset();
    _UNLOCK(-1);

    return 0;
}


[[nodiscard]] int sc::ptrscan::scan(
                    sc::opt & opts,
                    sc::opt_ptrscan & opts_ptrscan,
                    sc::map_area_set & ma_set,
                    worker_pool & w_pool,
                    const cm_byte flags) {

    /*
     *  NOTE: This method is full of goto's, but I can't think of a
     *        better approach. 5 layers of indentation is not a better
     *        approach.
     */

    int ret;
    bool run_err = false;


    //lock the scanner
    _LOCK(-1)

    //reset the pointer scan
    this->do_reset();


    //lock options
    ret = opts._lock();
    if (ret != 0) {
        run_err = true;
        goto _scan_ret;
    }

    //lock ptrscan options
    ret = opts_ptrscan._lock();
    if (ret != 0) {
        run_err = false;
        goto _scan_unlock_opts;
    }

    //lock the map areas set
    ret = ma_set._lock();
    if (ret != 0) {
        run_err = false;
        goto _scan_unlock_opts_ptrscan;
    }

    //check all necessary options have been set
    if (opts_ptrscan.get_target_addr().has_value() == false
        || opts_ptrscan.get_alignment().has_value() == false
        || opts_ptrscan.get_max_obj_sz().has_value() == false
        || opts_ptrscan.get_max_depth().has_value() == false) {

        sc_errno = SC_ERR_OPT_MISSING;
        run_err = true;
        goto _scan_unlock_all;
    }

    //setup the worker pool
    ret = w_pool.setup(opts, opts_ptrscan, *this, ma_set, flags);
    if (ret != 0) goto _scan_unlock_all;


    //for every depth level
    for (int i = 0; i < opts_ptrscan.get_max_depth().value(); ++i) {

        //scan the selected address space once
        ret = w_pool._single_run();
        if (ret != 0) goto _scan_unlock_all;

        //increment current depth
        ++this->cur_depth_level;
    }

    //flatten the tree
    ret = this->flatten_tree();
    if (ret != 0) goto _scan_unlock_all;


    _scan_unlock_all:
    ret = ma_set._unlock();
    if (ret != 0) run_err = true;

    _scan_unlock_opts_ptrscan:
    ret = opts_ptrscan._unlock();
    if (ret != 0) run_err = true;

    _scan_unlock_opts:
    ret = opts._unlock();
    if (ret != 0) run_err = true;
    
    _scan_ret:
    _UNLOCK(-1)
    return run_err ? -1 : 0;
}


[[nodiscard]] int sc::ptrscan::verify(
        sc::opt & opts, const sc::opt_ptrscan & opts_ptrscan) {

    bool valid;
    mc_session * session;


    //lock scanner
    _LOCK(-1)

    //check a target address is provided
    if (opts_ptrscan.get_target_addr().has_value() == false) {
        sc_errno = SC_ERR_OPT_MISSING;
        goto _verify_fail;
    }

    //check at least one session is provided
    if (opts.get_sessions().empty() == true) {
        sc_errno = SC_ERR_OPT_MISSING;
        goto _verify_fail;
    }

    //check there are chains to verify
    if (this->chains.empty() == true) {
        sc_errno = SC_ERR_NO_RESULT;
        goto _verify_fail;
    }

    //check chains were processed if read from disk
    if (this->chains[0].obj_node.has_value() == false) {
        sc_errno = SC_ERR_SHALLOW_RESULT;
        goto _verify_fail;
    }

    //for every chain
    for (int i = 0; i < this->chains.size(); ++i) {

        //delete this chain if it fails verification
        valid = is_chain_valid(opts_ptrscan.get_target_addr().value(),
                               this->chains[i],
                               *((mc_session *) opts.get_sessions()[0]));
        if (valid == false) {
            this->chains.erase(this->chains.begin() + i);
            --i;
        }
    } //end for every chain

    _UNLOCK(-1)
    return 0;

    _verify_fail:
    _UNLOCK(-1)
    return -1;
}


//fetch pointer chains
const std::vector<struct sc::ptrscan_chain> & sc::ptrscan::get_chains() const {
    return this->chains;    
}

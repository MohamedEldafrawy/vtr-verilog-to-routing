#ifndef NETLIST2_H
#define NETLIST2_H
/*
 * Summary
 * ========
 * This file defines the AtomNetlist class used to store and manipulate the primitive (or atom) netlist.
 *
 * Overview
 * ========
 * The netlist logically consists of several different components: Blocks, Ports, Pins and Nets
 * Each componenet in the netlist has a unique identifer (AtomBlockId, AtomPortId, AtomPinId, AtomNetId) used to
 * retireve information about it.
 *
 * Block
 * -----
 * A Block is the primitive netlist element (a node in the netlist hyper-graph). 
 * Blocks have various attributes (a name, a type etc.) and are associated with sets of
 * input/output/clock ports.
 *
 * Block related information can be retrieved using the block_*() member functions.
 *
 * Ports
 * -----
 * A Port is a (potentially multi-bit) group of signals which enter/exit a block.
 * For example, the two operands and output of an N-bit adder would logically be grouped as three ports. 
 * Ports have a specified bit-width which defines how many pins form the port.
 *
 * Port related information can be retrieved using the port_*() member functions.
 *
 * Pins
 * ----
 * Pins define single-bit connections between a port and a net.
 *
 * Pin related information can be retrieved using the pin_*() member functions.
 *
 * Nets
 * ----
 * Nets represent the connections between blocks (the edges of the netlist hyper-graph).
 * Each net has a single driver pin, and a set of sink pins.
 *
 * Net related information can be retrieved using the net_*() member functions.
 *
 * Usage
 * =====
 * This section provides usage examples for common use-cases.
 *
 * Walking the netlist
 * -------------------
 * To iterate over the whole netlist use the blocks() and/or nets() member functions:
 *
 *      AtomNetlist netlist;
 *
 *      //... initialize the netlist
 *
 *      //Iterate over all the blocks
 *      for(AtomBlockId blk_id : netlist.blocks()) {
 *          //Do something with each block
 *      }
 *
 *
 *      //Iterate over all the nets
 *      for(AtomNetId net_id : netlist.nets()) {
 *          //Do something with each net
 *      }
 *
 * To retrieve information about a netlist component call one of the associated member functions:
 *      
 *      //Print out each block's name
 *      for(AtomBlockId blk_id : netlist.blocks()) {
 *
 *          //Get the block name
 *          const std::string& block_name = netlist.block_name(blk_id);
 *
 *          //Print it
 *          printf("Block: %s\n", block_name.c_str());
 *      }
 *  
 * Note that the member functions are associated with the type of componenet (e.g. block_name() yields
 * the name of a block, net_name() yeilds the name of a net).
 *
 * Tracing cross-references
 * ------------------------
 * It is common to need to trace the netlist connectivity. The AtomNetlist allows this to be done 
 * efficiently by maintaining cross-references between the various netlist components.
 *
 * For example consider the case where we wish to find all the blocks associated with a particular net:
 *
 *      AtomNetId net_id;
 *
 *      //... Initialize net_id with the net of interest
 *
 *      //Iterate through each pin on the net to get the associated port
 *      for(AtomPinId pin_id : netlist.net_pins(net_id)) {
 *          
 *          //Get the port associated with the pin
 *          AtomPortId port_id = netlist.pin_port(pin_id);
 *
 *          //Get the block associated with the port
 *          AtomBlockId blk_id = netlist.port_block(port_id);
 *          
 *          //Print out the block name
 *          const std::string& block_name = netlist.block_name(blk_id);
 *          printf("Associated block: %s\n", block_name.c_str());
 *      }
 *
 * AtomNetlist also defines some convenience functions for common operations to avoid tracking the intermediate IDs
 * if they are not needed. The following produces the same result as above:
 *
 *      AtomNetId net_id;
 *
 *      //... Initialize net_id with the net of interest
 *
 *      //Iterate through each pin on the net to get the associated port
 *      for(AtomPinId pin_id : netlist.net_pins(net_id)) {
 *          
 *          //Get the block associated with the pin (bypassing the port)
 *          AtomBlockId blk_id = netlist.pin_block(pin_id);
 *          
 *          //Print out the block name
 *          const std::string& block_name = netlist.block_name(blk_id);
 *          printf("Associated block: %s\n", block_name.c_str());
 *      }
 *
 *
 * As another example, consider the inverse problem of identifying the nets connected as inputs to a particular block:
 *
 *      AtomBlkId blk_id;
 *
 *      //... Initialize blk_id with the block of interest
 *
 *      //Iterate through the ports
 *      for(AtomPortId port_id : netlist.block_input_ports(blk_id)) {
 *
 *          //Iterate through the pins
 *          for(AtomPinId pin_id : netlist.port_pins(port_id)) {
 *              //Retrieve the net
 *              AtomNetId net_id = netlist.pin_net(pin_id);
 *
 *              //Get its name
 *              const std::string& net_name = netlist.net_name(net_id);
 *              printf("Associated net: %s\n", net_name.c_str());
 *          }
 *      }
 *
 * Here we used the block_input_ports() method which returned an iterable range of all the import ports
 * associated with blk_id. We then used the port_pins() method to get iterable ranges of all the pins
 * associated with each port, from which we can find the assoicated net.
 *
 * Note that we used range-based-for loops in the above, we could also have written (more verbosely) using 
 * a conventional for loop and explicit iterators as follows:
 *
 *      AtomBlkId blk_id;
 *
 *      //... Initialize blk_id with the block of interest
 *
 *      //Iterat through the ports
 *      auto input_ports = netlist.block_input_ports(blk_id);
 *      for(auto port_iter = input_ports.begin(); port_iter != input_ports.end(); ++port_iter) {
 *
 *          //Iterate through the pins
 *          auto pins = netlist.pins(*port_iter);
 *          for(auto pin_iter = pins.begin(); pin_iter != pins.end(); ++pin_iter) {
 *              //Retrieve the net
 *              AtomNetId net_id = netlist.pin_net(*pin_iter);
 *
 *              //Get its name
 *              const std::string& net_name = netlist.net_name(net_id);
 *              printf("Associated net: %s\n", net_name.c_str());
 *          }
 *      }
 *
 * Creating the netlist
 * --------------------
 * The netlist can be created by using the create_*() member functions to create individual Blocks/Ports/Pins/Nets
 *
 * For instance to create the following netlist (where each block is the same type, and has an input port 'A' 
 * and output port 'B':
 *
 *      -----------        net1         -----------
 *      | block_1 |-------------------->| block_2 |
 *      -----------          |          -----------
 *                           |
 *                           |          -----------
 *                           ---------->| block_3 |
 *                                      -----------
 * We could do the following:
 *
 *      const t_model* blk_model = .... //Initialize the block model appropriately
 *
 *      AtomNetlist my_nl("my_netlist"); //Initialize the netlist with name 'my_netlist'
 *
 *      //Create the first block
 *      AtomBlockId blk1 = netlist.create_block("block_1", blk_model);
 *
 *      //Create the first block's output port
 *      AtomPortId blk1_out = netlist.create_port(blk1, "B");
 *
 *      //Create the net
 *      AtomNetId net1 = netlist.create_net("net1");
 *
 *      //Associate the net with blk1
 *      netlist.create_pin(blk1_out, 0, net_id, AtomPinType::DRIVER);
 *
 *      //Create block 2 and hook it up to net1
 *      AtomBlockId blk2 = netlist.create_block("block_2", blk_model);
 *      AtomPortId blk2_in = netlist.create_port(blk2, "A");
 *      netlist.create_pin(blk2_in, 0, net1, AtomPinType::SINK);
 *
 *      //Create block 2 and hook it up to net1
 *      AtomBlockId blk3 = netlist.create_block("block_3", blk_model);
 *      AtomPortId blk3_in = netlist.create_port(blk3, "A");
 *      netlist.create_pin(blk3_in, 0, net1, AtomPinType::SINK);
 *
 * Modifying the netlist
 * ---------------------
 * The netlist can also be modified by using the remove_*() member functions. If we wanted to remove
 * block_3 from the netlist creation example above we could do the following:
 *
 *      //Mark blk3 and any references to it invalid
 *      netlist.remove_block(blk3);
 *
 *      //Compress the netlist to actually remove the data assoicated with blk3
 *      // NOTE: This will invalidate all client held IDs (e.g. blk1, blk1_out, net1, blk2, blk2_in)
 *      netlist.compress();
 *
 * The resulting netlist connectivity now looks like:
 *
 *      -----------        net1         -----------
 *      | block_1 |-------------------->| block_2 |
 *      -----------                     -----------
 *
 * Note that until compress() is called any removed elements will have invalid IDs (e.g. AtomBlockId::INVALID()).
 * As a result after calling remove_block() (which invalidates blk3) we *then* called compress() to remove the
 * invalid IDs.
 *
 * Also note that compress() is relatively slow. As a result avoid calling compress() after every call to
 * a remove_*() function, and instead batch up calls to remove_*() and call compress() after a set of modifications 
 * have been applied.
 *
 * Verifying the netlist
 * ---------------------
 * Particuarliy after construction and/or modification it is a good idea to check that the netlist is in 
 * a valid and consistent state. This can be done with the verify() member function:
 *
 *      netlist.verify()
 *
 * If the netlist is not valid verify() will throw an exception, otherwise it returns true.
 *
 * Implementation
 * ==============
 * The netlist is stored in Struct-of-Arrays format rather than the more conventional Array-of-Structs.
 * This improves cache locality by keeping component attributes of the same type in contiguous memory.
 * This prevents unneeded member data from being pulled into the cache (since most code accesses only a few
 * attributes at a time this tends to be more efficient).
 *
 * Clients of this class pass nearly-opaque IDs (AtomBlockId, AtomPortId, AtomPinId, AtomNetId, AtomStringId) to retrieve
 * information. The ID is internally converted to a vector index to retrieve the required value from it's associated storage.
 *
 * By using nearly-opaque IDs we can change the underlying data layout as need to optimize performance/memory, without
 * distrupting client code.
 */
#include <vector>
#include <unordered_map>

#include "vtr_hash.h"
#include "vtr_range.h"
#include "vtr_logic.h"

#include "logic_types.h" //For t_model

#include "netlist2_fwd.h"


/*
 * Make various tuples hash-able for usin in std::unordered_map's
 *
 * C++11 does not natively support hashing std::tuple's even in the tuple's components
 * are hash-able.  We use vtr::hash_combine (see vtr_hash.h for details), to combine the
 * hash of the individual tuple components. 
 *
 * Note that it is legal to define template *specializations* in the std namespace, 
 * and makes the associated tuples behave like natively hash-able types.
 */
namespace std {
    template<>
    struct hash<std::tuple<AtomPortId,BitIndex>> {
        std::size_t operator()(const std::tuple<AtomPortId,BitIndex>& k) const {
            std::size_t seed = 0;
            vtr::hash_combine(seed, std::hash<AtomPortId>()(get<0>(k)));
            vtr::hash_combine(seed, std::hash<size_t>()(get<1>(k)));
            return seed;
        }
    };
    template<>
    struct hash<std::tuple<AtomStringId,AtomPortType>> {
        typedef std::underlying_type<AtomPortType>::type enum_type;
        std::size_t operator()(const std::tuple<AtomStringId,AtomPortType>& k) const {
            std::size_t seed = 0;
            vtr::hash_combine(seed, std::hash<AtomStringId>()(get<0>(k)));
            vtr::hash_combine(seed, std::hash<enum_type>()(static_cast<enum_type>(get<1>(k))));
            return seed;
        }
    };
    template<>
    struct hash<std::tuple<AtomBlockId,AtomStringId>> {
        std::size_t operator()(const std::tuple<AtomBlockId,AtomStringId>& k) const {
            std::size_t seed = 0;
            vtr::hash_combine(seed, std::hash<AtomBlockId>()(get<0>(k)));
            vtr::hash_combine(seed, std::hash<AtomStringId>()(get<1>(k)));
            return seed;
        }
    };
} //namespace std


class AtomNetlist {
    public: //Public types
        typedef std::vector<AtomBlockId>::const_iterator block_iterator;
        typedef std::vector<AtomPortId>::const_iterator port_iterator;
        typedef std::vector<AtomPinId>::const_iterator pin_iterator;
        typedef std::vector<AtomNetId>::const_iterator net_iterator;
        typedef std::vector<std::vector<vtr::LogicValue>> TruthTable;
    public:

        //Constructs a netlist
        // name - the name of the netlist
        AtomNetlist(std::string name="");

    public: //Public Accessors
        /*
         * Netlist
         */
        //Retrieve the name of the netlist
        const std::string&  netlist_name() const;

        /*
         * Block
         */
        //Returns the name of the specified block
        const std::string&          block_name          (const AtomBlockId id) const;

        //Returns the type of the specified block
        AtomBlockType               block_type          (const AtomBlockId id) const;

        //Returns the model associated with the block
        const t_model*              block_model         (const AtomBlockId id) const;

        //Returns the truth table associated with the block
        // Note that this is only non-empty for LUTs and Flip-Flops/latches.
        //
        // For LUTs the truth table stores the single-output cover representing the
        // logic function.
        //
        // For FF/Latches there is only a single entry representing the initial state
        const TruthTable&           block_truth_table   (const AtomBlockId id) const; 

        //Returns a range consisting of all the input ports associated with the specified block
        vtr::Range<port_iterator>   block_input_ports   (const AtomBlockId id) const;

        //Returns a range consisting of all the output ports associated with the specified block
        // Note this is typically only data ports, but some blocks (e.g. PLLs) can produce outputs
        // which are clocks.
        vtr::Range<port_iterator>   block_output_ports  (const AtomBlockId id) const;

        //Returns a range consisting of all the input clock ports associated with the specified block
        vtr::Range<port_iterator>   block_clock_ports   (const AtomBlockId id) const;

        /*
         * Port
         */
        //Returns the name of the specified port
        const std::string&          port_name   (const AtomPortId id) const;

        //Returns the width (number of bits) in the specified port
        BitIndex                    port_width  (const AtomPortId id) const;

        //Returns the block associated with the specified port
        AtomBlockId                 port_block  (const AtomPortId id) const; 

        //Returns the type of the specified port
        AtomPortType                port_type   (const AtomPortId id) const; 

        //Returns the set of valid pins associated with the port
        vtr::Range<pin_iterator>    port_pins   (const AtomPortId id) const;

        //Returns the pin (potentially invalid) associated with the specified port and port bit index
        AtomPinId port_pin   (const AtomPortId port_id, BitIndex port_bit) const;

        //Returns the net (potentially invalid) associated with the specified port and port bit index
        AtomNetId port_net   (const AtomPortId port_id, BitIndex port_bit) const;

        /*
         * Pin
         */
        //Returns the net associated with the specified pin
        AtomNetId   pin_net         (const AtomPinId id) const; 

        //Returns the pin type of the specified pin
        AtomPinType pin_type        (const AtomPinId id) const; 

        //Returns the port associated with the specified pin
        AtomPortId  pin_port        (const AtomPinId id) const;

        //Returns the port bit index associated with the specified pin
        BitIndex    pin_port_bit    (const AtomPinId id) const;

        //Returns the block associated with the specified pin
        AtomBlockId pin_block       (const AtomPinId id) const;


        /*
         * Net
         */
        //Returns the name of the specified net
        const std::string&          net_name    (const AtomNetId id) const; 

        //Returns a range consisting of all the pins in the net (driver and sinks)
        //The first element in the range is the driver (and may be invalid)
        //The remaining elements (potentially none) are the sinks
        vtr::Range<pin_iterator>    net_pins    (const AtomNetId id) const;

        //Returns the (potentially invalid) net driver pin
        AtomPinId                   net_driver  (const AtomNetId id) const;

        //Returns a (potentially empty) range consisting of net's sink pins
        vtr::Range<pin_iterator>    net_sinks   (const AtomNetId id) const;

        /*
         * Aggregates
         */
        //Returns a range consisting of all blocks in the netlist
        vtr::Range<block_iterator>  blocks  () const;

        //Returns a range consisting of all nets in the netlist
        vtr::Range<net_iterator>    nets    () const;
        
        /*
         * Lookups
         */
        //Returns the AtomBlockId of the specified block or AtomBlockId::INVALID() if not found
        //  name: The name of the block
        AtomBlockId find_block  (const std::string& name) const;

        //Returns the AtomPortId of the specifed port if it exists or AtomPortId::INVAILD() if not
        //  blk_id: The ID of the block who's ports will be checked
        //  name  : The name of the port to look for
        AtomPortId  find_port   (const AtomBlockId blk_id, const std::string& name) const;

        //Returns the AtomPinId of the specified pin or AtomPinId::INVALID() if not found
        //  port_id : The ID of the associated port
        //  port_bit: The bit index of the pin in the port
        AtomPinId   find_pin    (const AtomPortId port_id, BitIndex port_bit) const;

        //Returns the AtomNetId of the specified net or AtomNetId::INVALID() if not found
        //  name: The name of the net
        AtomNetId   find_net    (const std::string& name) const;

        /*
         * Utility
         */
        //Sanity check for internal consistency (throws an exception on failure)
        bool verify() const;

        //Indictes if the netlist has invalid entries due to modifications (e.g. from remove_*() calls)
        bool dirty() const;

        //Item counts and container info (for debugging)
        void print_stats() const;

    public: //Public Mutators
        /*
         * Note: all create_*() functions will silently return the appropriate ID if it has already been created
         */

        //Create or return an existing block in the netlist
        //  name        : The unique name of the block
        //  model       : The primitive type of the block
        //  truth_table : The single-output cover defining the block's logic function
        //                The truth_table is optional and only relevant for LUTs (where it describes the logic function)
        //                and Flip-Flops/latches (where it consists of a single entry defining the initial state).
        AtomBlockId create_block(const std::string name, const t_model* model, const TruthTable truth_table=TruthTable());

        //Create or return an existing port in the netlist
        //  blk_id      : The block the port is associated with
        //  name        : The name of the port (must match the name of a port in the block's model)
        AtomPortId  create_port (const AtomBlockId blk_id, const std::string& name);

        //Create or return an existing pin in the netlist
        //  port_id : The port this pin is associated with
        //  port_bit: The bit index of the pin in the port
        //  net_id  : The net the pin drives/sinks
        //  type    : The type of the pin (driver/sink)
        AtomPinId   create_pin  (const AtomPortId port_id, BitIndex port_bit, const AtomNetId net_id, const AtomPinType type);

        //Create an empty, or return an existing net in the netlist
        //  name    : The unique name of the net
        AtomNetId   create_net  (const std::string name); //An empty or existing net

        //Create a completely specified net from specified driver and sinks
        //  name    : The name of the net (Note: must not already exist)
        //  driver  : The net's driver pin
        //  sinsks  : The net's sink pins
        AtomNetId   add_net     (const std::string name, AtomPinId driver, std::vector<AtomPinId> sinks);

        /*
         * Note: all remove_*() will mark the associated items for removal, but will the items
         * will not be removed until compress() is called.
         */

        //Removes a block from the netlist. This will also remove the associated ports and pins.
        //  blk_id  : The block to be removed
        void remove_block   (const AtomBlockId blk_id);

        //Removes a net from the netlist. 
        //This will mark the net's pins as having no associated.
        //  net_id  : The net to be removed
        void remove_net     (const AtomNetId net_id);

        //Removes a connection betwen a net and pin. The pin is removed from the net and the pin
        //will be marked as having no associated net
        //  net_id  : The net from which the pin is to be removed
        //  pin_id  : The pin to be removed from the net
        void remove_net_pin (const AtomNetId net_id, const AtomPinId pin_id);

        //Compresses the netlist, removing any invalid and/or unreferenced
        //blocks/ports/pins/nets.
        //
        //This should be called after completing a series of netlist modifications 
        //(e.g. removing blocks/ports/pins/nets).
        //
        //NOTE: this invalidates existing IDs!
        void compress();

    private: //Private types

    private: //Private members
        /*
         * Lookups
         */
        //Returns the AtomStringId of the specifed string if it exists or AtomStringId::INVAILD() if not
        //  str : The string to look for
        AtomStringId find_string(const std::string& str) const;

        //Returns the AtomBlockId of the specifed block if it exists or AtomBlockId::INVAILD() if not
        //  name_id : The block name to look for
        AtomBlockId find_block(const AtomStringId name_id) const;

        //Returns the AtomPortId of the specifed port if it exists or AtomPortId::INVAILD() if not
        //  blk_id : The ID of the block who's ports will be checked
        //  name_id: The string ID of the port name to look for
        AtomPortId  find_port(const AtomBlockId blk_id, const AtomStringId name_id) const;

        //Returns the AtomNetId of the specifed port if it exists or AtomNetId::INVAILD() if not
        //  name_id: The string ID of the net name to look for
        AtomNetId find_net(const AtomStringId name_id) const;

        //Returns the model port of the specified port or nullptr if not
        //  port_id: The ID of the port to look for
        //  name   : The name of the port to look for
        //
        //Note that 'name' is required since this function may be called before the port has been fully
        //initialized
        const t_model_ports* find_model_port(const AtomPortId port_id) const;

        /*
         * Mutators
         */
        //Create or return the ID of the specified string
        //  str: The string whose ID is requested
        AtomStringId create_string(const std::string& str);

        //Removes a port from the netlist.
        //The port's pins are also marked invalid and removed from any associated nets
        //  port_id: The ID of the port to be removed
        void remove_port(const AtomPortId port_id);

        //Removes a pin from the netlist.
        //The pin is marked invalid, and removed from any assoicated nets
        //  pin_id: The ID of the pin to be removed
        void remove_pin(const AtomPinId pin_id);

        //Marks netlist components which have become redundant due to other removals
        //(e.g. ports with only invalid pins) as invalid so they will be destroyed during
        //compress()
        void remove_unused();

        /*
         * Netlist compression
         */
        //Removes invalid blocks and returns a mapping from old to new block IDs
        std::vector<AtomBlockId> clean_blocks();

        //Removes invalid ports and returns a mapping from old to new port IDs
        std::vector<AtomPortId> clean_ports();

        //Removes invalid pins and returns a mapping from old to new pin IDs
        std::vector<AtomPinId> clean_pins();

        //Removes invalid nets and returns a mapping from old to new net IDs
        std::vector<AtomNetId> clean_nets();

        //Re-builds cross-references held by blocks
        void rebuild_block_refs(const std::vector<AtomPortId>& port_id_map);

        //Re-builds cross-references held by ports
        void rebuild_port_refs(const std::vector<AtomBlockId>& block_id_map, const std::vector<AtomPinId>& pin_id_map);

        //Re-builds cross-references held by pins
        void rebuild_pin_refs(const std::vector<AtomPortId>& port_id_map, const std::vector<AtomNetId>& net_id_map);

        //Re-builds cross-references held by nets
        void rebuild_net_refs(const std::vector<AtomPinId>& pin_id_map);

        //Re-builds fast look-ups
        void rebuild_lookups();

        //Shrinks internal data structures to required size to reduce memory consumption
        void shrink_to_fit();

        /*
         * Sanity Checks
         */
        //Verify the internal data structure sizes match
        bool verify_sizes() const; //All data structures
        bool validate_block_sizes() const;
        bool validate_port_sizes() const;
        bool validate_pin_sizes() const;
        bool validate_net_sizes() const;
        bool validate_string_sizes() const;

        //Verify that internal data structure cross-references are consistent
        bool verify_refs() const; //All cross-references
        bool validate_block_port_refs() const;
        bool validate_port_pin_refs() const;
        bool validate_net_pin_refs() const;
        bool validate_string_refs() const;

        //Verify that fast-lookups are consistent with internal data structures
        bool verify_lookups() const;


        //Validates that the specified ID is valid in the current netlist state
        bool valid_block_id(AtomBlockId id) const;
        bool valid_port_id(AtomPortId id) const;
        bool valid_port_bit(AtomPortId id, BitIndex port_bit) const;
        bool valid_pin_id(AtomPinId id) const;
        bool valid_net_id(AtomNetId id) const;
        bool valid_string_id(AtomStringId id) const;

    private: //Private data

        //Netlist data
        std::string                 netlist_name_;  //Name of the top-level netlist
        bool                        dirty_;         //Indicates the netlist has invalid entries from remove_*() functions

        //Block data
        std::vector<AtomBlockId>             block_ids_;            //Valid block ids
        std::vector<AtomStringId>            block_names_;          //Name of each block
        std::vector<const t_model*>          block_models_;         //Architecture model of each block
        std::vector<TruthTable>              block_truth_tables_;   //Truth tables of each block
        std::vector<std::vector<AtomPortId>> block_input_ports_;    //Input ports of each block
        std::vector<std::vector<AtomPortId>> block_output_ports_;   //Output ports of each block
        std::vector<std::vector<AtomPortId>> block_clock_ports_;    //Clock ports of each block

        //Port data
        std::vector<AtomPortId>             port_ids_;      //Valid port ids
        std::vector<AtomStringId>           port_names_;    //Name of each port
        std::vector<AtomBlockId>            port_blocks_;   //Block associated with each port
        std::vector<std::vector<AtomPinId>> port_pins_;     //Pins associated with each port

        //Pin data
        std::vector<AtomPinId>      pin_ids_;       //Valid pin ids
        std::vector<AtomPortId>     pin_ports_;     //Type of each pin
        std::vector<BitIndex>       pin_port_bits_; //The ports bit position in the port
        std::vector<AtomNetId>      pin_nets_;      //Net associated with each pin

        //Net data
        std::vector<AtomNetId>              net_ids_;   //Valid net ids
        std::vector<AtomStringId>           net_names_; //Name of each net
        std::vector<std::vector<AtomPinId>> net_pins_;  //Pins associated with each net

        //String data
        // We store each unique string once, and reference it by an StringId
        // This avoids duplicating the strings in the fast look-ups (i.e. the look-ups
        // only store the Ids)
        std::vector<AtomStringId>   string_ids_;    //Valid string ids
        std::vector<std::string>    strings_;       //Strings

    private: //Fast lookups

        std::unordered_map<AtomStringId,AtomBlockId>                        block_name_to_block_id_;
        std::unordered_map<std::tuple<AtomBlockId,AtomStringId>,AtomPortId> block_id_port_name_to_port_id_;
        std::unordered_map<std::tuple<AtomPortId,BitIndex>,AtomPinId>       pin_port_port_bit_to_pin_id_;
        std::unordered_map<AtomStringId,AtomNetId>                          net_name_to_net_id_;
        std::unordered_map<std::string,AtomStringId>                        string_to_string_id_;
};

#endif

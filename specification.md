#             Lanes protocol - draft specification

### Protocol data unit  (Packet/Frame) structure 
 
```   

+----------------------------------------------------------------+----------------+
|                ~14 Bytes of Ethernet frame                     | 38 bytes raw v4|
|                                                                | 58 bytes raw v6|
+----------------------------------------------------------------+ 58 bytes UDP v4|
|                                                                | 98 bytes UDP v6|
|                ~20 Bytes of IP header                          |                |
|         (Optional UDP encapsulation - 20-40 more bytes)        |  Un-encrypted  |
|                                                                |    Header      |
+----------------------------------------------------------------+                |
|               32 bits (4 Bytes) Tunnel ID                      |                |
+-------------------------------+-------------------------------------------------+
|       8 bits reserved         | 8 bits packet type             |                |
+-------------------------------+--------------------------------+    6 Bytes     |
|                32 bits(4 bytes) packet value                   |   Encrypted    |
+----------------------------------------------------------------+     Header     |
|                                                                |      And       |
|                                                                |   User data    |
|                                                                |                |
|                                                                |                |
|                                                                |                |
|                                                                |                |
|                         USER DATA                              |                |
|                                                                |                |
|                                                                |                |
|                                                                |                |
|                                                                |                |
|                                                                |                |
|                                                                |                |
|                                                                |                |
|                                                                |                |
|                                                                |                |
|                                                                |                |
|                                                                |                |
|                                                                |                |
|                                                                |                |
|                                                                |                |
|                                                                |                |
+----------------------------------------------------------------+----------------+

```
### Fields 

    - IPv4,IPv6 and UDP    
      * If the user has configured an access control list permitting or denying a specific 
        IPv4 or IPv6 peer or ethernet source or destination,the packet should be processed accordingly.
        If there is no such list configured the packet is to be processed without any ethernet
        or IPv4/IPv6 processing.
        
      * In order to traverse NAT or prevent discarding of packets by intermediate network devices,
        UDP encapsulation support is mandated. 
        However UDP protocol details are to be ignored with prejudice.The only acceptable use of UDP
        is to prevent intermediate network devices from dropping the packet. 
        Unfortunately network devices that do not comply with existing RFC exist,they make packet forwarding decisions
        based on Layer4 properties of a packet therefore we have to accommodate their malpractice.
     
    - Tunnel ID
      * 32 bit numeric identifier. this will associate the packet in question with unique
        and specific crystallographic processing and network or local destination.
        
      * If there is no matching tunnel ID configured on the receiver,the packet
        is to be discarded without any further processing. 
        
      * Transmission or egress tunnels are to disregard and be agnostic towards the content and nature
        of the data they are transmitting.
        
      * A tunnel ID cannot be bound to more than one type of data source.
        for example ethernet data sources cannot be bound to the same tunnel ID 
        as an IPv6 data source.
        
      * A tunnel ID can be used by many physical or logical transmission tunnel instances so long 
        as their data source is of the same nature.
        This enables the formation of a 1-to-1,1-to-many and many-to-many tunnel using the same tunnel ID 
        for network and crystallographic association. 
        
      * Data sources can include but are not limited to IPv4/IPv6 tunnels,ethernet virtual interfaces,
        other physical interfaces, named pipes,unix sockets , RPC sockets or websockets. 
        Implementations are free to pick and choose what local data sources they wish to support for
        transmission. the protocol makes no mandate or restriction on the nature of transported data.
        
    - Reserved 
      * the first byte (8bits) will mark the beginning of encrypted data.
      
      * this field has no current meaning.
      
      * possible future uses are to measure the amount of padding inserted for the purposes of ciphers that
        require such padding.
        
    - Packet type and value
      * these two fields are not to be used separately. if the type does not need a value then the value
        is to be set to 0. 
        
      * Type will tell the receiver what properties the current packet has and if any special treatment is required.
      
      * Value tells the receiver what values should be used when processing the packet in accordance with it's packet type.
      
        - Currently defined packet type-value pairs.
        
        Type 0 - No further processing will be done
                Value: 0/None
                  * This packet will be accepted and forwarded accordingly without any additional processing
                    or manipulation of data.
                    
        Type 1 - Fragmented packet - first packet 
                Value: 0 - 16,777,215, this is the fragment ID 
                  * Every fragmented packet before fragmentation is assigned this ID.
                  
                  * Receivers are to hold to this packet until either all fragments are assembled 
                    or the fragment buffer of the receiver is full and the packet is discarded.
                    
                  * This is a hard limit on how many unique fragmented packets there can be. 
                  
                  * if the tunnel is transmitting more than 16,777,215 fragmented packets in the time
                    it takes for the receiver to collect and re-assemble them, it will have to start
                    re-using fragment ID's. 
                    If this is a problem,the solution will be to stop sending data larger than what
                    the tunnel's MTU can accommodate.
                    
        Type 2 - Fragmented packet - subsequent packet.
                Value Byte 1:  0 - 255
                  * 0 would be the second in sequence of the  fragmented  packet, 1 the third and so on.
                  
                  * A given packet can only be fragmented into 255 pieces, this will impose a hard limit
                    on the source and destination tunnel MTU's. 
                    The maximum fragmentable packet size being 354960 for a minimal tunnel MTU of 1392 bytes.
                Value Bytes 2-4: 0 - 16,777,215
                
                  * Fragment ID, this will tell the receiver what packet the current fragment belongs to.
                  
                  * If no matching fragment ID is found, the packet is to be discarded immediately.
                  
   Type  3-224 - unassigned reserved for future use.
                  * this range will be used for further development and improvement of the protocol
                  
  Type 224-240 - signaling of traffic compression - reserved.
                Value: unassigned
                  * Various data compression schemes can be signaled using these ranges.
                  
  Type 240-254 - reserved for testing.
                  * Should Implementations require custom signaling or messaging they can use this range.
                  
      Type 255 - Discard packet. 
                Value: 0 Drop packet ,disregard content.
                
                Value: Non-zero 
                  * non zero value is set when injection of data for either testing,security or 
                    crystallographic reasons is desired.
                    
                  * one use could be simulating fake response/replies using random data to mitigate
                    certain traffic size/pattern correlation attacks.
                    
                  * a non zero value is useful to inform the receiver, to respond a certain way, should it
                    support this optional feature.
                   
### Protocol requirements
  
  - Uni-directional tunnels
       * Compliant implementations of the Lanes protocol cannot associate a tunnel ID with more than one direction of
         traffic. 
         This applies only to the forwarding of traffic.
         Among other possibilities, Public key cryptography key exchange will need bi-directional communication.
         
       * Encryption key used to transmit data may not be re-used to receive or decrypt data on another tunnel
         configured under the same instance of the application.
         
       * A tunnel must function in complete independence of all other tunnels' state or property.
           For example, A user should be able to configure a dedicated machine for the establishment 
           and forwarding of an egress only tunnel.
           
  - Data agnostic tunnels
       * it is a requirement for implementations of the Lanes protocol to disregard and be unaware of the data they are transporting.
       
       * this does not include the nature of the data source but the nature of the data itself.
       
       * the nature of the data source(for example a tuntap interface in linux or another physical interface)
         is relevant only for the purposes of proper encapsulation and decapsulation of the data as well as
         for establishing unique tunnel ID and crystallographic associations. 
         
  - Tunnel ID based forwarding
       * Traffic can be accepted or rejected based on layer 3 (IPv4 or IPv6) properties,however
         for the purposes of the Lanes protocol,it may not be processed any further than an accept or drop decision based
         on the layer 3 properties of the traffic.
         
       * Traffic may not be processed any differently, either when it comes to the transmission or encryption
         of the payload in relation to the layer 3 properties of the receiver.
         
       * The tunnel ID and the packet's payload type and value properties are the only factor that should decide how traffic
         is processed after reception.

### Other comments   
    * At this time this draft specification,the operational specification of the current implementation as well as
      the implementation itself are under heavy development. if you're reading this your input is very much welcome.
      
    * Since the tunnel ID's are 32bit in length, implementations and users are free to use them using the dotted-decimal
      format popularized by ipv4,however this is not a requirement and there is no relation with lanes tunnel ID's and IPv4.
      
    * Current efforts are focused at implementing the specification already discussed in this document.
      However there are many ideas afloat. here are some 
        *  Formation of MPLS-like LSP and label stacking using tunnel ID's and signaling of such labels using type-value
           payload header field.
           This will help users or admins who would like to support forwarding of traffic to other lanes speakers 
           based on tunnel ID's without requiring repetitive decryption/encryption on every hop.
           Example:
           Network A may wish to send traffic to a lanes speaker B,that same network A might want to send traffic
           to a different speaker C but the shortest path maybe through speaker B. the use of mpls-style labeling will 
           solve this problem without requiring lanes implementations to implement MPLS or to use it for data encapsulation.
           This may be supported as an optional feature in the future.
           This will also not require the users or admins of lanes to know or understand MPLS. this is just an idea at this time.
         * Out-of-band signaling scheme might be needed. 
            However if the host supports unix sockets, they can easily be used to administer a lanes instance
            remotely using a unix tunnel formed using that same lanes instance. 
           this is useful for SDN style orchestration and control of tunnel traffic by a master 'controller'
           
     * A lot of ideas in this document are inspired by existing technologies and protocols, this may lead some to believe 
       Lanes is just re-inventing the wheel. rest assured most of what this document describes is not existing technology.
       where that is not true, the re-invention of the figurative wheel is only there to improve how the wheel is used to transport it's payload.
       
     * Lanes forms tunnels not VPNs. it is stated that way to clarify that a Lanes tunnel does not need to be encrypted
       therefore it cannot guarantee privacy when configured in such a way. Also both "Virtual" and "Network" ,while they
       sound nice, they are merely restrictions on how lanes would be implemented and operated therefore the term VPN can only
       apply if lanes is configured to form a VPN.
       
     * Active queue management is a very important entry in the todo list , various queue management algorithms are being considered.
       if implemented things like per-tunnel QoS will be possible. not sure how good of an idea this is since upper layer protocols
       can already do this,regardless it will be an optional protocol feature. 
     
***************************************************************************************************************************

## - OpenLanes Operational Specification

TX-IO
thread count == $cpucount/2 (default; user configurable)

polls all local data sources
decides queue based on data source
queues data without further processing

TX
thread count == $cpucount --fixed
pops data blocks from queue
fragments as needed
fills out payload header
encrypts accordingly
transmits using the appropriate associated outgoing interface

RX IO 

thread count == $cpucount/2 (default; user configurable)
polls all local configured NIC's
receives and validates frames
decides queue based on payload header tunnel id field
queues data without further processing

RX

thread count == $cpucount -- fixed
pops data blocks from queue
decrypts packets
re-assembles fragments as needed
decides what to do with decrypted data based type and type value
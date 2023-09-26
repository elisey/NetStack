# Net stack

Custom network stack (partly compliant with OSI model) with wired and wireless communication support.

It is implemented from scratch with C++ for ARM STM32 microcontrollers.

- At the lower level, it supports bus and point-to-point topologies.
- At the network level, it provides transparent addressing of all devices, splitting and reassembling of long packets
- At the transport layer, it supports streaming data simultaneously over several sockets with delivery control and resending in case of losses.
- At the MAC level it supports wired (duplex) and wireless (NRF24L01) connections
- The additional module provides monitoring of network integrity, the presence of neighboring devices in the network, and automatic construction of a routing table.

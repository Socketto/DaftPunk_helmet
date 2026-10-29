extern unsigned char Init_MCP(unsigned char Address);
extern void SetOutputs(unsigned char Address, unsigned char PortA, unsigned char PortB);



#define MCP_COUNTS  8

extern unsigned char MCPAddresses[];
extern unsigned char MCPPresent[];
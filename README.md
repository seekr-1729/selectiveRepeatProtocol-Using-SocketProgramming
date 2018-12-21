# selectiveRepeatProtocol-Using-SocketProgramming


Compile both client_src.c and server_src.c with gcc :

      _> gcc server_src.c -o server
      _> gcc client_src.c -o client
      
run as : ./server and then in a different cmd ./client

Description:

Transfers the file in.txt from client to server using the selective repeat sliding window protocol. The server takes as input the drop rate which is used to randomly drop packets to effectively simulate the protocol.

The output file is to be given as out.txt in server_src.c

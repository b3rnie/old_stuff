import java.io.*;
import java.net.*;

public class Test{
	public static String socks_host;
	public static int socks_port=1080;
	
	public static void main(String args[])throws IOException{
		if(args.length!=1){
			System.out.println("usage: <java Test hostname>");
			return;
		}
		socks_host=args[0];
		
		System.out.println("***** TESTING SOCKS4 CONNECT *****");
		socks4_connect();
	
		System.out.println("\n***** TESTING SOCKS4 BIND *****");
		socks4_bind();
		
		System.out.println("\n***** TESTING SOCKS5 CONNECT *****");
		socks5_connect();
		
		System.out.println("\n***** TESTING SOCKS5 BIND *****");
		socks5_bind();
		
		System.out.println("\n***** TESTING SOCKS5 UDP RELAY *****");
		socks5_udp_relay();
	}
	
	public static void socks4_bind()throws IOException{
		System.out.println("trying to connect...");
		Socket sock=new Socket(socks_host,socks_port);
		
		byte msg[]=new byte[9];
		msg[0]=4;
		msg[1]=0x02;
		msg[2]=0;
		msg[3]=0;
		msg[4]=0;
		msg[5]=0;
		msg[6]=0;
		msg[7]=0;
		msg[8]=0;
		
		System.out.println("writing request...");
		sock.getOutputStream().write(msg);
		
		System.out.println("reading response...");
		DataInputStream dis=new DataInputStream(sock.getInputStream());
		
		dis.readUnsignedByte();
		if(dis.readUnsignedByte()!=90){
			System.out.println("request failed...");
			return;
		}
		
		int port=dis.readUnsignedShort();
		
		byte addr[]=new byte[4];
		dis.readFully(addr);
		
		System.out.println("trying to connect to bound port: "+port);
		Socket sock2=new Socket(InetAddress.getByAddress(addr),port);

		byte foo[]=new byte[8];
		dis.readFully(foo);
		if(foo[1]!=90){
			System.out.println("request failed");
			return;
		}
		test_pipe(sock2,sock);
	}

	public static void socks4_connect()throws IOException{
		System.out.println("binding serversocket...");
		ServerSocket ssock=new ServerSocket();
		ssock.bind(null);
		
		System.out.println("trying to connect...");
		Socket sock=new Socket(socks_host,socks_port);
		
		byte msg[]=new byte[9];
		msg[0]=4;
		msg[1]=0x01;
		msg[2]=(byte)(ssock.getLocalPort()>>8);
		msg[3]=(byte)ssock.getLocalPort();
		
		byte addr[]=ssock.getInetAddress().getAddress();
		System.arraycopy(addr,0,msg,4,addr.length);
		msg[8]=0;
		
		System.out.println("writing request...");
		sock.getOutputStream().write(msg);
		
		System.out.println("reading response...");
		byte buff[]=new byte[100];
		sock.getInputStream().read(buff);

		if(buff[1]!=90){
			System.out.println("request failed...");
			return;
		}
		

		System.out.println("accepting connection...");
		Socket sock2=ssock.accept();
		
		test_pipe(sock2,sock);
	}
	
	public static void socks5_connect()throws IOException{
		System.out.println("binding serversocket...");
		ServerSocket ssock=new ServerSocket();
		ssock.bind(null);
		
		System.out.println("trying to connect...");
		Socket sock=new Socket(socks_host,socks_port);
		
		if(!socks5_auth(sock))
			return;
		
		byte msg2[]=new byte[10];
		msg2[0]=5;
		msg2[1]=0x01;
		msg2[2]=0x00;
		msg2[3]=0x01;
		byte addr[]=ssock.getInetAddress().getAddress();
		System.arraycopy(addr,0,msg2,4,addr.length);
		
		msg2[8]=(byte)(ssock.getLocalPort()>>8);
		msg2[9]=(byte)ssock.getLocalPort();
				
		System.out.println("writing request...");
		sock.getOutputStream().write(msg2);
		
		System.out.println("reading response...");
		byte buff[]=new byte[100];
		sock.getInputStream().read(buff);
		
		System.out.println("accepting connection...");
		Socket sock2=ssock.accept();
		
		test_pipe(sock2,sock);

	}

	public static boolean socks5_auth(Socket sock)throws IOException{
		byte msg[]=new byte[2];
		msg[0]=5;
		msg[1]=0;
		
		System.out.println("writing auth request...");
		sock.getOutputStream().write(msg);
		
		System.out.println("reading auth reply...");
		byte buff[]=new byte[100];
		sock.getInputStream().read(buff);
		if(buff[1]!=0){
			System.out.println("auth failed...");
			return false;
		}
		
		return true;
	}
	
	public static void socks5_bind()throws IOException{
		System.out.println("trying to connect...");
		Socket sock=new Socket(socks_host,socks_port);
		
		if(!socks5_auth(sock))
			return;
		
		byte msg[]=new byte[10];
		msg[0]=5;
		msg[1]=0x02;
		msg[2]=0x00;
		msg[3]=0x01;
		msg[4]=0;
		msg[5]=0;
		msg[6]=0;
		msg[7]=0;
		msg[8]=0;
		msg[9]=0;
		
		System.out.println("writing request...");
		sock.getOutputStream().write(msg);
		
		System.out.println("reading response...");
		DataInputStream dis=new DataInputStream(sock.getInputStream());
		
		dis.readUnsignedByte();
		if(dis.readUnsignedByte()!=0x00){
			System.out.println("request failed...");
			return;
		}
		dis.readUnsignedByte();
		dis.readUnsignedByte();
		
		byte addr[]=new byte[4];
		dis.readFully(addr);
		
		int port=dis.readUnsignedShort();
		
		System.out.println("trying to connect to bound port: "+port);
		Socket sock2=new Socket(InetAddress.getByAddress(addr),port);
		
		byte foo[]=new byte[10];
		dis.readFully(foo);
		if(foo[1]!=0x00){
			System.out.println("request failed...");
			return;
		}
		
		test_pipe(sock2,sock);
	}
	
	public static void socks5_udp_relay(){
		System.out.println("NOT IMPLEMENTED");
	}

	public static void test_pipe(Socket sock1,Socket sock2)throws IOException{
		System.out.println("trying to pipe some data...");
		
		byte buff1[]=new byte[10];
		byte buff2[]=new byte[10];
		for(int i=0;i<10;i++)
			buff1[i]=(byte)i;
		
		sock1.getOutputStream().write(buff1);
		sock2.getInputStream().read(buff2);
		
		sock2.getOutputStream().write(buff1);
		sock1.getInputStream().read(buff2);
	}
}

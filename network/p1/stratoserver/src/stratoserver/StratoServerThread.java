package stratoserver;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.util.LinkedList;
import java.util.Queue;

public class StratoServerThread extends Thread {
	
	protected StratoServer stratmain;
	protected InputStream is;
    protected OutputStream os;
	protected Socket threadsocket;
	private boolean AUTHSTATUS=false;
	private byte[] barray=new byte[2];
	int blength;
	private String ipayload,ipayload2;
	private String remaddress;
	protected DataInputStream dis;
	protected DataOutputStream dos;
	int passtry;
	int TOKEN;
	protected Queue<Integer> hashq;
	
	public StratoServerThread(Socket s, StratoServer stratmain) {
		this.stratmain=stratmain;
		threadsocket=s;
		hashq=new LinkedList<Integer>();
	}
	public void addtohashq(int i) {
		hashq.add(i);
	}

	public void run() {
		
		remaddress=threadsocket.getRemoteSocketAddress().toString();
		
		try {
			is=threadsocket.getInputStream();
			os=threadsocket.getOutputStream();
			dis=new DataInputStream(is);
			dos=new DataOutputStream(os);
		} catch (IOException e) {
			System.out.println("error in is/os: "+e);
			e.printStackTrace();
		}
		
		 while (true) {
			 ipayload="";ipayload2="";
			 try {
				 for(int i=0;i<2;i++) {/**read 2 bytes**/
					 barray[i]=dis.readByte();
				 }
				 if(barray[0]==-1 && barray[1]==-1) {/**************client disconnect***************/
					 System.out.println("Disconnecting from "+remaddress);
					 disconnect();
					 break;
				 }
				 if(barray[0]==0) {/********************************0-AUTHENTICATION*****************************************/
					 blength=dis.readInt();
					 byte[] bb=dis.readNBytes(blength);
					 for(int i=0;i<blength;i++) {
						 ipayload+=(char)bb[i];
					 }
					 System.out.println("login attempt wiht uname: "+ipayload);
					 passtry=0;
					 try {
						 threadsocket.setSoTimeout(15000);
					 } catch (SocketException e) {
						 e.printStackTrace();
					 }
					 while(passtry<3) {
						 dos.writeByte(0);dos.writeByte(1);
						 if(passtry==0) {
							 dos.writeInt(7);dos.writeBytes("pwd_req");
						 }
						 else {
							 dos.writeInt(18);dos.writeBytes("incorrect_password");
						 }
						 dos.flush();
						 ipayload2="";
						 dis.readByte();dis.readByte();
						 blength=dis.readInt();
						 bb=dis.readNBytes(blength);
						 for(int i=0;i<blength;i++) {
							 ipayload2+=(char)bb[i];
						 }
						 System.out.println("pass"+passtry+": "+ipayload2);
						 BufferedReader bufred=new BufferedReader(new FileReader("users"));
						 String bufline=bufred.readLine();
						 boolean match=false;
						 while(bufline!=null) {
							 if(bufline.split(" ")[0].equals(ipayload) && bufline.split(" ")[1].equals(ipayload2)) {
								 match=true;
								 break;
							 }
							 bufline=bufred.readLine();
						 }
						 if(match) {
							 bufred.close();
							 try {
								 threadsocket.setSoTimeout(0);
							 } catch (SocketException e) {
								 System.out.println(e);
							 }
							 AUTHSTATUS=true;
							 TOKEN=(threadsocket.getLocalPort()+ipayload.hashCode());
							 dos.writeByte(0);dos.writeByte(3);dos.writeInt(4);dos.writeInt(TOKEN);
							 dos.flush();
							 dos.writeByte(0);dos.writeByte(4);dos.writeInt(4);dos.writeInt(4455);
							 dos.flush();
							 break;
						 }else {
							 passtry++;
						 }
						 bufred.close();
					 }
					 if(!AUTHSTATUS) {
						 dos.writeByte(0);dos.writeByte(2);dos.writeInt(10);dos.writeBytes("wrong_pass");
						 System.out.println("Disconnecting from "+remaddress);
						 disconnect();
						 break;
					 }
					 
				 }else if(barray[0]==1) {/********************************1-QUERY*****************************************/
					 System.out.println("IN QUERY PHASE");
					 blength=dis.readInt();
					 dis.readByte();
					 int ttoken=dis.readInt();
					 if(ttoken!=TOKEN) {
						 String s="Token error";
						 System.out.println("TOKEN ERROR at : "+threadsocket.getRemoteSocketAddress());
						 dos.writeByte(1);dos.writeByte(3);dos.writeInt(s.length());dos.writeBytes(s);
						 dos.flush();
						 disconnect();
						 stratmain.getsft(TOKEN).disconnect();
						 break;
					 }
					 try {
						Thread.sleep(1500);
					} catch (InterruptedException e) {
						System.out.println(e);
					}
					 if(barray[1]==0) {/*********************apod***********************/
						 System.out.println("IN APOD PHASE");
						 try {
							Thread.sleep(1500);
						} catch (InterruptedException e1) {
							System.out.println(e1);
						}
						 stratmain.getsft(TOKEN).addjobs(0, "https://api.nasa.gov/planetary/apod?api_key=FCRklbRrllQC1pevtk1vAQp24VTOCBzYM41OG7vF", this);
						 System.out.println("ADDING APOD JOB");
						 try {
							Thread.sleep(3000);
							while(hashq.isEmpty())Thread.sleep(5000);
							int hash=hashq.poll();
							System.out.println("HASH OF FILE: "+hash);
							dos.writeByte(1);dos.writeByte(2);dos.writeInt(4);dos.writeInt(hash);
							dos.flush();
							System.out.println("HASH OF APOD FILE SENT");
						} catch (InterruptedException e) {
							System.out.println(e);
						}
						 
					 }else if(barray[1]==1) {/*********************mars***********************/
						 System.out.println("IN MARS PHASE");
						 try {
							Thread.sleep(1500);
						} catch (InterruptedException e1) {
							System.out.println(e1);
						}
						 stratmain.getsft(TOKEN).addjobs(1, "https://api.nasa.gov/insight_weather/?api_key=FCRklbRrllQC1pevtk1vAQp24VTOCBzYM41OG7vF&feedtype=json&ver=1.0", this);
						 System.out.println("ADDING MARS JOB");
						 try {
							Thread.sleep(3000);
							if(hashq.isEmpty())Thread.sleep(5000);
							int hash=hashq.poll();
							System.out.println("HASH OF FILE: "+hash);
							dos.writeByte(1);dos.writeByte(2);dos.writeInt(4);dos.writeInt(hash);
							dos.flush();
							System.out.println("HASH OF MARS FILE SENT");
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
						 
					 }
				 }else {
					 System.out.println("Unknown message.");
				 }
				 
			} catch (SocketTimeoutException e) {
				System.out.println("Time out at address "+ remaddress);
				disconnect();
				break;
			}catch (IOException e) {
				//System.out.println(e);
				disconnect();
				break;
			} 
		 }
		
		
	}
	
	 private void disconnect() {
		 try {
			dos.writeByte(-1);dos.writeByte(-1);
			dis.close();
			dos.close();
			is.close();
			os.close();
			threadsocket.close();
		} catch (IOException e) {
			System.out.println("Connection was closed by client at: "+remaddress);
		}
		 
	 }

}

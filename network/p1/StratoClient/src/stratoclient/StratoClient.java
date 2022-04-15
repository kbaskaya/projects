package stratoclient;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.ConnectException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Scanner;

public class StratoClient {
	
	public static final String DEFAULT_SERVER_ADDRESS = "localhost";
    public static final int DEFAULT_SERVER_PORT = 4444;
    private static Socket s;
    private static Socket sf;
    private static InputStream is,isf;
    private static OutputStream os,osf;
    private static String serverAddress="localhost";
    private static int serverPort=4444;
    private static DataInputStream dis,disf;
	private static DataOutputStream dos,dosf;
	private static String[] sarray,sarrayf;
	protected static boolean auth=false;
	protected static int authToken;
	static String name;
	private static byte[] barray=new byte[2];
	private static byte[] barrayf=new byte[2];
	static int blength,bflength;
	static String line=new String();
	static Scanner scanner=new Scanner(System.in);
	static String incomingstring=new String();
	static int serverhash;
	

	public static void main(String[] args) throws ConnectException {
		try {
			s= new Socket(serverAddress,serverPort);
			is=s.getInputStream();
			os=s.getOutputStream();
			dis=new DataInputStream(is);
			dos=new DataOutputStream(os);
			System.out.println("Connected to StratoServer at: "+serverAddress+":"+serverPort);
		} catch (Exception e) {
			System.out.println("Error while connecting. Exiting.");
			return;
		}
		System.out.println("Type \"HELP!\" for help.");
		while(true) {
			line=scanner.nextLine();
			usercommand(line);
			if(line.equals("QUIT!"))break;
		}

	}
	
	
	
	public static void disconnect() {
		try {
			System.out.println("Disconnecting from StratoServer.");
			dos.close();
			dis.close();
			os.close();
			is.close();
			s.close();
			dosf.close();
			disf.close();
			osf.close();
			isf.close();
			sf.close();
		} catch (IOException e) {
			System.out.println("error: "+e);
		}
	}
	
	
	
	
	public static void usercommand(String ss) {
		switch(ss.split("!")[0]) {
		/**************************************MARS!***********************************************/
		case "MARS":
			if(!auth) {
				System.out.println("Not logged in.");
				break;
			}
			try {
				dos.writeByte(1);dos.writeByte(1);dos.writeInt(1);dos.writeByte(0);dos.writeInt(authToken);
				barray[0]=dis.readByte();barray[1]=dis.readByte();blength=dis.readInt();
				if(barray[1]==2) {
					serverhash=dis.readInt();
				}else if(barray[1]==3) {
					byte[] bb=dis.readNBytes(blength);
					for(int i=0;i<blength;i++) {
						 incomingstring+=(char)bb[i];
					 }
					System.out.println(incomingstring);
					disconnect();
					break;
				}
				int size=disf.readInt();
				String marsdata="";
				byte[] bb=disf.readNBytes(size);
				for(int i=0;i<size;i++) {
					marsdata+=(char)bb[i];
				 }
			    if(serverhash==marsdata.hashCode())System.out.println(marsdata);
			    else System.out.println("Downloaded data is corrupted. Try again.");
			    
			} catch (IOException e1) {
				System.out.println(e1);
			}
			break;
		/**************************************APOD!***********************************************/
		case "APOD":
			if(!auth) {
				System.out.println("Not logged in.");
				break;
			}
			try {
				dos.writeByte(1);dos.writeByte(0);dos.writeInt(1);dos.writeByte(0);dos.writeInt(authToken);
				barray[0]=dis.readByte();barray[1]=dis.readByte();blength=dis.readInt();
				if(barray[1]==2) {
					serverhash=dis.readInt();
				}else if(barray[1]==3) {
					byte[] bb=dis.readNBytes(blength);
					for(int i=0;i<blength;i++) {
						 incomingstring+=(char)bb[i];
					 }
					System.out.println(incomingstring);
					disconnect();
					break;
				}
				int size=disf.readInt();
				OutputStream imgout = new BufferedOutputStream(new FileOutputStream("apod.jpg"));
			    for (int i=0; i<size; i++) {
			        imgout.write(disf.read());
			        //System.out.println("size:"+size+" cur:"+i);
			    }
			    File f=new File("apod.jpg");
			    if(serverhash==f.hashCode())System.out.println("Image downloaded as apod.jpg");
			    else System.out.println("Downloaded file is corrupted. Try again.");
			    imgout.close();
			} catch (IOException e1) {
				System.out.println(e1);
			}
			break;
		/**************************************HELP!***********************************************/
		case "HELP":
			System.out.println("LOGIN! uname");
			System.out.println("\tLogin by providing username \"uname\"");
			System.out.println("APOD!");
			System.out.println("\tDownload the Astronomy Picture of the Day.");
			System.out.println("MARS!");
			System.out.println("\tGet the atmospheric pressure of a random sol.");
			System.out.println("QUIT!");
			System.out.println("\tDisconnect and quit.");
			break;
		/**************************************QUIT!***********************************************/
		case "QUIT":
			try {
				dos.writeByte(-1);dos.writeByte(-1);
				dos.flush();
				disconnect();
			} catch (IOException e) {
				System.out.println("error in os.write: "+e);
			}
			break;
		/**************************************LOGIN!***********************************************/
		case "LOGIN":
			try {
				if(auth) {
					System.out.println("Already logged in.");
					break;
				}
				sarray=ss.split(" ");
				if(sarray.length==2) {
					dos.writeByte(0);dos.writeByte(0);dos.writeInt(sarray[1].length());dos.writeBytes(sarray[1]);
					dos.flush();
					name=sarray[1];
					while(true) {
						barray[0]=dis.readByte();
						barray[1]=dis.readByte();
						blength=dis.readInt();
						
						if(barray[1]==1) {
							byte[] bb=dis.readNBytes(blength);
							for(int i=0;i<blength;i++) {
								 incomingstring+=(char)bb[i];
							 }
							if(incomingstring.equals("wrong_pass"))System.out.println("Wrong password.");
							System.out.println("Enter password:");
							line=scanner.nextLine();
							dos.writeByte(0);dos.writeByte(0);dos.writeInt(line.length());dos.writeBytes(line);
						}else if(barray[1]==2) {
							byte[] bb=dis.readNBytes(blength);
							for(int i=0;i<blength;i++) {
								 incomingstring+=(char)bb[i];
							 }
							System.out.println("Wrong login info.");
							disconnect();
							System.exit(0);
						}else if(barray[1]==3) {
							authToken=dis.readInt();
							auth=true;
							System.out.println("Logged in as: "+name);
							barray[0]=dis.readByte();
							barray[1]=dis.readByte();
							blength=dis.readInt();blength=dis.readInt();
							sf=new Socket(serverAddress, blength);
							isf=sf.getInputStream();
							osf=sf.getOutputStream();
							disf=new DataInputStream(isf);
							dosf=new DataOutputStream(osf);
							dosf.writeInt(authToken);
							break;
						}else {
							byte[] bb=dis.readNBytes(blength);
							for(int i=0;i<blength;i++) {
								 incomingstring+=(char)bb[i];
							 }
							System.out.println("Unknown incoming message: "+incomingstring);
							break;
						}
					}
				}else {
					System.out.println("Wrong input format for login.");
				}
			} catch (IOException e) {
				System.out.println("error in os.write: "+e);
			}
			break;
		/**************************************INVALID-COMMAND***********************************************/	
		default:
				System.out.println("invalid command. commands: LOGIN!/QUIT!");
			
		}
	}

}

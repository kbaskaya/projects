package stratoserver;

import java.io.File;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.util.HashMap;

public class StratoServer {
	
	protected HashMap<Integer ,StratoFileThread> hmap;
	
	private ServerSocket mainSocket;
	private ServerSocket fileSocket;
	public static final int DEF_PORT=4444;
	public static final int DEF_FILE_PORT=4455;
	
	
	public StratoServer(int port) {
		
		System.out.println();
		System.out.println("STRATOSERVER.JAVA");
		System.out.println();
		hmap= new HashMap<Integer ,StratoFileThread>();
		
		
		File myFile = new File("users");
		try {
			if(myFile.createNewFile()) {
				System.out.println("new database created: "+myFile.getName());
			}else System.out.println("user database found");
		} catch (IOException e) {
			System.out.println("error during reading users: "+e);
		}
		
		
		
		
		try {
			mainSocket= new ServerSocket(port);
			fileSocket= new ServerSocket(DEF_FILE_PORT);
			System.out.println("socket opened at port:" + mainSocket.getLocalPort());
		} catch (IOException e) {
			System.out.println("error while opening socket: "+ e);
		}
		
		
		
		try {
			mainSocket.setSoTimeout(50);
			fileSocket.setSoTimeout(50);
		} catch (SocketException e1) {
			System.out.println("error at settimeout: "+e1);
		}
		
		
		
		while(true) {
			listenaccept();
		}
		
	}
	
	private void listenaccept() {
		
		Socket s;
		Socket fs;
		try {
			s=mainSocket.accept();
			System.out.println("accepted connection: socket: "+s.getRemoteSocketAddress()+" port: "+s.getPort());
			StratoServerThread sthread=new StratoServerThread(s,this);
			sthread.start();
		} catch (SocketTimeoutException et) {
			//System.out.println("error while accepting: "+et);
		} catch (IOException e) {
			System.out.println("error while accepting: "+e);
		}
		try {
			fs=fileSocket.accept();
			System.out.println("accepted file connection: socket: "+fs.getRemoteSocketAddress()+" port: "+fs.getPort());
			StratoFileThread sfthread=new StratoFileThread(fs,this);
			sfthread.start();
		} catch (SocketTimeoutException et) {
			
		} catch (IOException e) {
			System.out.println("error while accepting: "+e);
		}
	}
	
	
	public void updatehmap(int tOKEN, StratoFileThread sft) {
		hmap.put(tOKEN, sft);
	}
	public StratoFileThread getsft(int tOKEN) {
		return hmap.get(tOKEN);
	}

}


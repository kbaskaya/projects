package stratoserver;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.MalformedURLException;
import java.net.Socket;
import java.net.URL;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Random;
import java.util.Scanner;


public class StratoFileThread extends Thread {
	
	protected StratoServerThread sst;
	protected StratoServer stratmain;
	protected Socket socket;
	protected InputStream is;
    protected OutputStream os;
    protected DataInputStream dis;
	protected DataOutputStream dos;
	int TOKEN;
	Queue<JobNode> jobq;
	private String remad;
	
	public StratoFileThread(Socket s, StratoServer ss) {
		this.socket=s;
		this.stratmain=ss;
		jobq=new LinkedList<JobNode>();
	}
	
	
	public void run() {
		
		try {
			is=socket.getInputStream();
			os=socket.getOutputStream();
			dis=new DataInputStream(is);
			dos=new DataOutputStream(os);
			remad=socket.getRemoteSocketAddress().toString();
			System.out.println("File socket connected to port: "+socket.getPort());
			int ttoken;
			ttoken=dis.readInt();
			TOKEN=ttoken;
			stratmain.updatehmap(TOKEN, this);
		} catch (IOException e) {
			System.out.println("error in is/os: "+e);
			e.printStackTrace();
		}
		
		while(true) {
			checkjobs();
		}
	}
	
	public void checkjobs() {
		if(!jobq.isEmpty()) {
			JobNode job=jobq.poll();
			if(job.j==0) {//apodapodapodapodapodapodapodapodapodapodapodapodapodapodapodapodapod
				String s;
				URL url;
				try {
					url = new URL(job.link);
					Scanner scanner = new Scanner(url.openStream());
					String inc = scanner.useDelimiter("\\Z").next();
					if (inc.contains("hdurl")) {
						s = inc.substring(inc.indexOf("hdurl"));
						s=s.substring(s.indexOf("https"));
						s=s.substring(0, s.indexOf(",")-1);
					}else {
						s = inc.substring(inc.indexOf("url"));
						s=s.substring(s.indexOf("https"));
						s=s.substring(0, s.indexOf(",")-1);
					}
					URL imgurl = new URL(s);
				    InputStream in = new BufferedInputStream(imgurl.openStream());
				    OutputStream out = new BufferedOutputStream(new FileOutputStream("apod.jpg"));
				    int x=0;
			        File f=new File("apod.jpg");
				    for ( int i; (i = in.read()) != -1; ) {
				        out.write(i);
				        x++;
				    }
			        dos.writeInt((int)f.length());dos.flush();
			        System.out.println("\tFILE SIZE SENT");
			        System.out.println(f.length());
				    in=new BufferedInputStream(new FileInputStream("apod.jpg"));
				    out = new BufferedOutputStream(os);
				    for ( int i; (i = in.read()) != -1;) {
				    	dos.write(i);
				    }
				    dos.flush();
				    System.out.println("\tFILE SENT");
				    in.close();
				    job.sst.addtohashq(f.hashCode());
				} catch (MalformedURLException e1) {
					e1.printStackTrace();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}else {//marsmarsmarsmarsmarsmarsmarsmarsmarsmarsmarsmarsmarsmarsmarsmarsmars
				URL url;
				try {
					url = new URL(job.link);
					String s,ss,fin;
					Scanner scanner = new Scanner(url.openStream());
					String inc = scanner.useDelimiter("\\Z").next();
					
					s=inc.substring(inc.indexOf("sol_keys"));
					s=s.substring(s.indexOf("["));
					s=(String) s.subSequence(0, s.indexOf("]"));
					s=s.substring(s.indexOf("\""));
					ArrayList<String> al=new ArrayList<String>();
					while(s.indexOf(",")!=-1) {
						ss=s.substring(s.indexOf("\"")+1);
						ss=(String) ss.subSequence(0, ss.indexOf("\""));
						al.add(ss);
						s=s.substring(s.indexOf(",")+1);
					}
					s=s.substring(s.indexOf("\"")+1);
					s=(String) s.subSequence(0, s.indexOf("\""));
					al.add(s);
					System.out.println(al);
					Random rand= new Random();
					int x=rand.nextInt(al.size());
					String picked=al.get(x);
					
					fin="SOL \""+picked+"\" INFO: ";
					s=inc.substring(inc.indexOf(picked));
					s=s.substring(s.indexOf("PRE"));
					s=s.substring(s.indexOf("{"));
					s=(String) s.subSequence(0, s.indexOf("}"));
					s=s.substring(s.indexOf("\""));
					System.out.println(s);
					while(s.indexOf(",")!=-1) {
						ss=s.substring(s.indexOf("\"")+1);
						ss=(String) ss.subSequence(0, ss.indexOf(","));
						fin+=ss+" ";
						s=s.substring(s.indexOf(",")+1);
					}
					s=s.substring(s.indexOf("\"")+1);
					fin+=s;
					fin=fin.replace("\n", "");
					fin=fin.replace("\t", "");
					fin=fin.replace("\"", "");
					dos.writeInt(fin.length());dos.flush();
					dos.writeBytes(fin);dos.flush();
					job.sst.addtohashq(fin.hashCode());
				} catch (MalformedURLException e) {
					e.printStackTrace();
				} catch (IOException e) {
					e.printStackTrace();
				}
				
			}
		}
	}
	public void addjobs(int i,String s,StratoServerThread sst) {
		JobNode jn = new JobNode(i,s,sst);
		jobq.add(jn);
	}
	public void receiveServerThread (StratoServerThread sst) {
		this.sst=sst;
	}
	
	public void disconnect() {
		try {
			dos.writeByte(-1);dos.writeByte(-1);
			dis.close();
			dos.close();
			is.close();
			os.close();
			socket.close();
		} catch (IOException e) {
			System.out.println("Connection was closed by client at: "+remad);
		}
	}
	
	private class JobNode{
		int j;//query type APOD0 or MARS1
		String link;//link to the nasa api
		StratoServerThread sst;//job issuing client
		public JobNode (int j, String l,StratoServerThread s ) {
			this.j=j;
			link=l;
			sst=s;
		}
	}

}

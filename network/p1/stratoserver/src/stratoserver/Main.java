package stratoserver;

public class Main {

	public static void main(String[] args) {
		
		System.out.println("Initializing StratoServer.");
		
		int port;
		if(args.length !=1) {
			System.out.println("will listen on default port:4444");
			port=4444;
		}else {
			port=Integer.parseInt(args[0]);
			System.out.println("will listen on port:"+port);
		}
		
		StratoServer stratoserver = new StratoServer(port);

	}

}

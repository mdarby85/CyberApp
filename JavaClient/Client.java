/** 
* file:Client.java
* author: Group 2 
* Date Modified: 11/23/19
* This program is the default client for MapZest
*/
package client;

import javax.naming.AuthenticationException;
import java.io.*;
import java.net.Socket;
import java.util.InputMismatchException;
import java.util.Scanner;
import java.util.Locale;

public class Client {
    /*Change to match other group's server*/
    private static String SERVERNAME = "localhost";
    /*Select port for Socket*/
    private static int PORT = 12348;
    private static int MAX_TRIES = 4;
    private static int MAX_BUFFER_SIZE = 200;
    private static Locale ENGLISH = Locale.forLanguageTag("en");
    
    /*
     * main
     * No command line parameters, server and port are hard-coded
     */
    public static void main(String[] args) throws Exception {
	try{
            Socket s = new Socket(SERVERNAME, PORT);
            InputStream in = s.getInputStream();
            OutputStream out = s.getOutputStream();
            byte[] bytesToSend;
	    /*Max buffer size*/
            byte[] bytesReceived = new byte[MAX_BUFFER_SIZE];
            
	    /*Set up scanner*/
            Scanner reader = new Scanner(System.in);
	    reader.useDelimiter("\\n");
            
	    System.out.println("Welcome to Group 2's MAPZEST!");
            User myUser = authenticate(reader, out, in);
            /*If the user exceeded max attempts, terminate*/
	    if (!myUser.getAuthenticated()) {
                throw new AuthenticationException("Max Tries Exceeded");
            }
            String c="A";
	    Menu.displayMenu(myUser);
            while( !(c=reader.next()).equals("H")){
                try{
                    bytesToSend = Menu.handleOption(c.charAt(0), myUser, reader);
                    out.write(bytesToSend);
                    in.read(bytesReceived,0,Menu.getExpectedReturnSize(c.charAt(0)));
                    Menu.handleResponse(c.charAt(0),myUser,bytesReceived);
		    Menu.displayMenu(myUser);
                }catch (IllegalArgumentException ex){
                    System.out.println("Invalid Choice, try again.");
                }
            }
        } catch (AuthenticationException ex){
            System.out.println(ex.getMessage());
        } catch (ConnectException ex){
	    System.out.println("Connection to " + SERVERNAME + " at port " + PORT + " could not be found.");
	}
        out.close();
        in.close();
        s.close();
    }
	
    /**
    *@param reader The scanner used to retrieve input 
    *@param out The output stream
    *@param in The socket's input stream
    *@return the User that is authenticated
    * This function authenticates a user
    */
    private static User authenticate(Scanner reader, OutputStream out, InputStream in) throws Exception{
        User myUser = new User();
        boolean valid = false;
        byte[] buf = new byte[MAX_BUFFER_SIZE];
        int numTries = 1;
	/*Limit number of tries*/
        while(!valid && numTries <= MAX_TRIES){
            System.out.println("Enter an email");
            String email = reader.nextLine();
            System.out.println("Enter a Password");
            String password = reader.nextLine();
            if(email.length() >= 32 || password.length() <= 1 || password.length() >= 32 ){
                System.out.println("Invalid email/password input, try again.");
		System.out.println(MAX_TRIES - numTries " attempts remaining");
		    numTries++;
            }
            else {
                myUser.setEmail(email);
                myUser.setPassword(password);
                out.write(DataProtocolEncoder.signIn(myUser));
                in.read(buf, 0, 65);
                if (buf[0] == 1) {
                    valid = true; /*Authenticate*/
                    myUser.setSignInToken(DataProtocolDecoder.retrieveToken(buf));
		    System.out.println("my Token is " + myUser.getToken());
                    myUser.setAuhenticated(true);
                } else {
                    System.out.println("Invalid Username/Password Pair, try again");
                    System.out.println(MAX_TRIES - numTries " attempts remaining");
		    numTries++;
                }
            }
        }
        return myUser;
    }
}

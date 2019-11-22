package client;

import javax.naming.AuthenticationException;
import java.io.*;
import java.net.Socket;
import java.util.InputMismatchException;
import java.util.Scanner;

public class Client {
    private static String SERVERNAME = "localhost";
    private static int PORT = 12348;
    private static int MAX_TRIES = 3;

    public static void main(String[] args) throws Exception {
        Socket s = new Socket(SERVERNAME, PORT);
        InputStream in = s.getInputStream();
        OutputStream out = s.getOutputStream();
        byte[] bytesToSend;
        byte[] bytesReceived = new byte[200];
        Scanner reader = new Scanner(System.in);
        try {
            System.out.println("Welcome to Group 2's MAPZEST!");
            User myUser = authenticate(reader, out, in);
            if (!myUser.getAuthenticated()) {
                throw new AuthenticationException("Max Tries Exceeded");
            }
            Menu.displayMenu(myUser);
            String c="A";
            while( !(c=reader.next()).equals("H")){
                try{
                    bytesToSend = Menu.handleOption(c.charAt(0), myUser, reader);
                    out.write(bytesToSend);
                    in.read(bytesReceived,0,Menu.getExpectedReturnSize(c.charAt(0)));
                    Menu.handleResponse(c.charAt(0),myUser,bytesReceived);
                }catch (IllegalArgumentException ex){
                    System.out.println("Invalid Input: " + ex.getMessage());
                }
            }
        } catch (AuthenticationException ex){
            System.out.println(ex.getMessage());
        }
        out.close();
        in.close();
        s.close();
    }

    private static User authenticate(Scanner reader, OutputStream out, InputStream in) throws Exception{
        User myUser = new User();
        boolean valid = false;
        byte[] buf = new byte[65];
        int numTries = 0;
        while(!valid && numTries <= MAX_TRIES){
            System.out.println("Enter an email");
            String email = reader.nextLine();
            System.out.println("Enter a Password");
            String password = reader.nextLine();
            if(email.length() >= 32 || password.length() <= 1 || password.length() >= 32 ){
                System.out.println("Invalid email/password input, try again.");
            }
            else {
                myUser.setEmail(email);
                myUser.setPassword(password);
                out.write(DataProtocolEncoder.signIn(myUser));
                in.read(buf, 0, 65);
                if (buf[0] == '1') {
                    valid = true;
                    myUser.setSignInToken(DataProtocolDecoder.retrieveToken(buf));
		    System.out.println("my Token is " + myUser.getToken());
                    myUser.setAuhenticated(true);
                } else {
                    System.out.println("Invalid Username/Password Pair, try again");
                    numTries++;
                    System.out.println("Try " + numTries + " of " + MAX_TRIES);
                }
            }
        }
        return myUser;
    }
}

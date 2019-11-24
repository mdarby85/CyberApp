/**
 * file: DataProtocolDecoder.java
 * author: Group 2
 * Date Modified: 11/23/19
 * This program is the default decoder for MapZest
 */
package client;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.*;
public abstract class DataProtocolDecoder {

    private static final byte GOOD_RESPONSE = 1;
    private static final byte DELIMITER = (byte)'\0';

    /**
     * Retrieve Token
     * @param token Encoded message
     * @return The User's SignIn Token
     */
    public static SignInToken retrieveToken(byte[] token){
        ByteBuffer buf = ByteBuffer.wrap(token);
        System.out.println("Getting token");
        if(buf.get() == GOOD_RESPONSE){
            byte[] mytoken = new byte[64];
            buf.get(mytoken,0,64);
            return new SignInToken(mytoken);
        }else{
            return null;
        }
    }

    /**
     * Turns an encoded message to a location
     * @param loc A location to be decoded
     * @return The Location
     */
    public static Location retrieveLocation(byte[] loc){
        ByteBuffer buf = ByteBuffer.wrap(loc);
        if(buf.get() == GOOD_RESPONSE){
            Location loc1 = new Location(buf.getFloat(),buf.getFloat());
            return loc1;
        }else{
            return null;
        }
    }

    /**
     * Get a list of friends from an an encoded message
     * @param friendList Encoded message
     * @return An array of friends
     */
    public static ArrayList<String> getFriends(byte[] friendList){
        ByteBuffer buf = ByteBuffer.wrap(friendList);
        ArrayList<String> myList = new ArrayList<>();
        StringBuilder myBuilder = new StringBuilder();
        if(buf.get() == GOOD_RESPONSE){
            int numList = buf.getInt();
            byte c;
            for(int i=0;i<numList;i++) {
                while (((c = buf.get()) != DELIMITER)) {
                    System.out.println((char)c);
                    myBuilder.append((char)c);
                }
                myList.add(myBuilder.toString());
                myBuilder.setLength(0);
            }
            return myList;
        }else{
            return null;
        }
    }

    /**
     * Get the response of an encoded message
     * @param response Return the response's first byte
     * @return
     */
    public static byte getResponse(byte[] response){
        return response[0];
    }
}

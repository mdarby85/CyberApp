package client;

import java.nio.ByteBuffer;
import java.util.ArrayList;

public abstract class DataProtocolDecoder {

    private static final byte GOOD_RESPONSE = 1;
    private static final byte DELIMITER = (byte)'\0';

    public static SignInToken retrieveToken(byte[] token){
        ByteBuffer buf = ByteBuffer.wrap(token);
        if(buf.get() == GOOD_RESPONSE){
            byte[] mytoken = new byte[64];
            buf.get(mytoken,0,64);
            return new SignInToken(mytoken);
        }else{
            return null;
        }
    }

    public static Location retrieveLocation(byte[] loc){
        ByteBuffer buf = ByteBuffer.wrap(loc);
        if(buf.get() == GOOD_RESPONSE){
            return new Location(buf.getFloat(),buf.getFloat());
        }else{
            return null;
        }
    }

    public static ArrayList<String> getFriends(byte[] friendList){
        ByteBuffer buf = ByteBuffer.wrap(friendList);
        ArrayList<String> myList = new ArrayList<>();
        StringBuilder myBuilder = new StringBuilder();
        if(buf.get() == GOOD_RESPONSE){
            int numList = buf.getInt();
            char c;
            for(int i=0;i<numList;i++) {
                while ((c = buf.getChar()) != DELIMITER) {
                    myBuilder.append(c);
                }
                myList.add(myBuilder.toString());
                myBuilder.setLength(0);
            }
            return myList;
        }else{
            return null;
        }
    }

    public static byte getResponse(byte[] response){
        return response[0];
    }
}

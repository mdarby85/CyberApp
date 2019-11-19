package client;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;

public abstract class DataProtocolEncoder {

    private static final Charset ENC = StandardCharsets.US_ASCII;
    private static final byte CODE_SIGN_IN = 0;
    private static final byte CODE_LOC_GET = 1;
    private static final byte CODE_LOC_UPDATE = 2;
    private static final byte CODE_LOC_FRIEND = 3;
    private static final byte CODE_FRIEND_GET = 4;
    private static final byte CODE_FRIEND_SEND = 5;
    private static final byte CODE_FRIEND_ACCEPT = 6;
    private static final byte CODE_FRIEND_REMOVE = 7;
    private static final byte TERMINATE_CONNECTION = 8;
    private static final byte DELIMITER = (byte)'\0';

    public static byte[] signIn(User myUser) throws IOException {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        out.write(CODE_SIGN_IN);
        out.write(myUser.getEmail().getBytes(ENC));
        out.write(DELIMITER);
        out.write(myUser.getPassword().getBytes(ENC));
        return out.toByteArray();
    }

    public static byte[] getLocation(User myUser) throws IOException{
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        out.write(CODE_LOC_GET);
        out.write(myUser.getToken().getToken());
        ByteBuffer.allocate(4).putFloat(myUser.getLocation().getLattitude()).array();
        out.write(ByteBuffer.allocate(4).putFloat(myUser.getLocation().getLattitude()).array());
        out.write(ByteBuffer.allocate(4).putFloat(myUser.getLocation().getLongitude()).array());
        return out.toByteArray();
    }

    public static byte[] updateLocation(User myUser) throws IOException{
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        out.write(CODE_LOC_UPDATE);
        out.write(myUser.getToken().getToken());
        ByteBuffer.allocate(4).putFloat(myUser.getLocation().getLattitude()).array();
        out.write(ByteBuffer.allocate(4).putFloat(myUser.getLocation().getLattitude()).array());
        out.write(ByteBuffer.allocate(4).putFloat(myUser.getLocation().getLongitude()).array());
        return out.toByteArray();
    }

    public static byte[] getFriendLocation(User myUser, String userID) throws IOException{
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        out.write(CODE_LOC_FRIEND);
        out.write(myUser.getToken().getToken());
        out.write(userID.getBytes(ENC));
        out.write(DELIMITER);
        return out.toByteArray();
    }

    public static byte[] getFriendRequest(User myUser) throws IOException{
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        out.write(CODE_FRIEND_GET);
        out.write(myUser.getToken().getToken());
        return out.toByteArray();
    }

    public static byte[] sendFriendRequest(User myUser, String friendUserID) throws IOException{
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        out.write(CODE_FRIEND_SEND);
        out.write(myUser.getToken().getToken());
        out.write(friendUserID.getBytes(ENC));
        out.write(DELIMITER);
        return out.toByteArray();
    }

    public static byte[] acceptFriendRequest(User myUser, String friendUserID) throws IOException{
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        out.write(CODE_FRIEND_ACCEPT);
        out.write(myUser.getToken().getToken());
        out.write(friendUserID.getBytes(ENC));
        out.write(DELIMITER);
        return out.toByteArray();
    }

    public static byte[] removeFriend(User myUser, String friendUserID) throws IOException{
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        out.write(CODE_FRIEND_REMOVE);
        out.write(myUser.getToken().getToken());
        out.write(friendUserID.getBytes(ENC));
        out.write(DELIMITER);
        return out.toByteArray();
    }

    public static byte[] terminateConnection() throws IOException{
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        out.write(TERMINATE_CONNECTION);
        return out.toByteArray();
    }
}

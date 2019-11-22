package client;

import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.math.BigInteger;
public class User {

    private static final Charset ENC = StandardCharsets.US_ASCII;
    private ArrayList<String> friends;
    private boolean isAuthenticated;
    private SignInToken signInToken;
    private String email;
    private String passwordHash;
    private Location loc;

    public User(){
        isAuthenticated = false;
        friends = new ArrayList<String>();
        this.loc = new Location(0,0);
    }

    public void setPassword(String pwd) throws NoSuchAlgorithmException {
        MessageDigest md5 = MessageDigest.getInstance("SHA-256");
        byte[] digest = md5.digest(pwd.getBytes(ENC));
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 32; ++i) {
            sb.append(Integer.toHexString((digest[i] & 0xFF) | 0x100).substring(1, 3));
        }
        this.passwordHash = sb.toString();
    }

    public void setEmail(String email){
        this.email = email;
    }
    public String getPassword(){
        return passwordHash;
    }

    public void setAuhenticated(boolean isAuthenticated){
        this.isAuthenticated = isAuthenticated;
    }

    public boolean getAuthenticated(){
        return isAuthenticated;
    }

    public void setLocation(Location loc){
        this.loc = loc;
    }

    public Location getLocation(){
        return loc;
    }

    public void setSignInToken(SignInToken token){
        this.signInToken = token;
    }

    public SignInToken getToken(){
        return signInToken;
    }

    public String getEmail(){
        return email;
    }
}

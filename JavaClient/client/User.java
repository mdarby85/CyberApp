/**
 * file:User.java
 * author: Group 2 
 * Date Modified: 11/23/19
 * This class represents a MapZestUser
 */
package client;

import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.math.BigInteger;

/**
 *@version 1.0
 * Represents a MapZest User
 */
public class User {

    private static final Charset ENC = StandardCharsets.US_ASCII;
    private ArrayList<String> friends;
    private boolean isAuthenticated;
    private SignInToken signInToken;
    private String email;
    private String passwordHash;
    private Location loc;

    /**
     * Default Constructor
     * Default User is given a location of 0,0
     */
    public User(){
        isAuthenticated = false;
        friends = new ArrayList<String>();
        this.loc = new Location(0,0);
    }

    /**
     * Generate a password using the first 32 bytes of SHA-256
     * @param pwd pwd to be generated
     * @throws NoSuchAlgorithmException
     */
    public void setPassword(String pwd) throws NoSuchAlgorithmException {
        MessageDigest md5 = MessageDigest.getInstance("SHA-256");
        byte[] digest = md5.digest(pwd.getBytes(ENC));
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 32; ++i) {
            sb.append(Integer.toHexString((digest[i] & 0xFF) | 0x100).substring(1, 3));
        }
        this.passwordHash = sb.toString();
    }

    /**
     * Default setter for email
     * @param email Email to be set
     */
    public void setEmail(String email){
        this.email = email;
    }

    /**
     * Gets the password
     * @return hashed password
     */
    public String getPassword(){
        return passwordHash;
    }

    /**
     * Sets authentication of the User
     * @param isAuthenticated
     */
    public void setAuhenticated(boolean isAuthenticated){
        this.isAuthenticated = isAuthenticated;
    }

    /**
     * Gets the authentication of the suer
     * @return the authentication of the user
     */
    public boolean getAuthenticated(){
        return isAuthenticated;
    }

    /**
     * Sets the location of the user
     * @param loc Location of the user
     */
    public void setLocation(Location loc){
        this.loc = loc;
    }

    /**
     * Gets the location of the user
     * @return User's location
     */
    public Location getLocation(){
        return loc;
    }

    /**
     * Sets the sign-in token of the user
     * @param token sign-in token 
     */
    public void setSignInToken(SignInToken token){
        this.signInToken = token;
    }

    /**
     * Gets the sign-in token
     * @return sign-in token
     */
    public SignInToken getToken(){
        return signInToken;
    }

    /**
     * Gets the email of the user
     * @return The email of the user
     */
    public String getEmail(){
        return email;
    }
}

/**
 * file:SigninToken.java
 * author: Group 2
 * Date Modified: 11/23/19
 * This class represents a sign-in token
 */
package client;
import java.util.Arrays;

public class SignInToken {
    byte[] token;

    /**
     * Default constructor for signin token
     * @param token Sign-in token's byte representation
     */
    public SignInToken(byte[] token){
        this.setToken(token);
    }

    /**
     * Get the token
     * @return the Sign-in's token byte representation
     */
    public byte[] getToken() {
        return token;
    }

    /**
     *  Sets the token to a new token
     * @param token The new sign-in token
     */
    public final void setToken(byte[] token) {
        this.token = Arrays.copyOf(token,token.length);
    }
}

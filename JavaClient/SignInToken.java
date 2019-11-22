package client;
import java.util.Arrays;

public class SignInToken {
    byte[] token;

    public SignInToken(byte[] token){
        this.setToken(token);
    }

    public byte[] getToken() {
        return token;
    }

    public final void setToken(byte[] token) {
        this.token = Arrays.copyOf(token,token.length);
    }
}

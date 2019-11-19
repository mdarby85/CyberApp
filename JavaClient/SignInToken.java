package client;

public class SignInToken {
    byte[] token;

    public SignInToken(byte[] token){
        this.token = token;
    }

    public byte[] getToken() {
        return token;
    }

    public final void setToken(byte[] token) {
        this.token = token;
    }
}

import { nanoid } from "nanoid";

const state: {[key: string]: number}= {};
//this generates a unique identifier as a nonce, and sets it to the value of "state", and returns it.
const genState = () => { const s = nanoid(); state[s] = 1; return s}
const runtime = useRuntimeConfig()

//constructs the URl for the login, where the user is sent when they're logged in
export const loginRedirectUrl = () => `${runtime.ISSUER}authorize?response_type=id_token&response_mode=form_post&client_id=${runtime.AUTH0_CLIENTID}&scope=openid%20email&redirect_uri=${encodeURIComponent(runtime.BASEURL!+"api/callback")}&nonce=${genState()}`
//cosntructs URL for logging out, where user is sent after they're logged out
export const logoutRedirectUrl = (id_token: string) => `${runtime.ISSUER}oidc/logout?id_token_hint=${id_token}&post_logout_redirect_uri=${encodeURIComponent(runtime.BASEURL!+"api/logoutcallback")}&nonce=${genState()}`


// Nonce -> Number used once

//after verifying the nonce used in the cookie, delete it and return it.
export const verifyNonce = (nonce: string) => {
    if (state[nonce]) {
        delete state[nonce]
        return true
    }
    return false
}
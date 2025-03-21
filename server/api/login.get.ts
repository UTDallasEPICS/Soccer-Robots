import { loginRedirectUrl } from "./auth0"

//Upon being called to login, send the user to the redirect URL
export default defineEventHandler(async event => {
    await sendRedirect(event, loginRedirectUrl() || "")
})
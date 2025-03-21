import { logoutRedirectUrl } from "./auth0";

//Upon being called to logout, send the user to the redirect URL
export default defineEventHandler(async event => {
  //get their token from the cookie to log them out
  const id_token = getCookie(event, "srtoken")
  await sendRedirect(event, logoutRedirectUrl(id_token as string) || "")
})
//code to clear the cookies upon logging out, and redirect user
export default defineEventHandler(async event => {
  setCookie(event, "srtoken", "")
  setCookie(event, "sruser", "")
  await sendRedirect(event, "/")
})
import { defineEventHandler, setCookie, sendRedirect } from 'h3'

export default defineEventHandler(async (event) => {
  // 1️⃣ Delete the auth cookie by setting it to empty and expired
  setCookie(event, 'sruser', '', {
    maxAge: 0,             // expire immediately
    httpOnly: true,        // optional: more secure
    path: '/',             // must match the cookie path
    secure: process.env.NODE_ENV === 'production',
  })

  // 2️⃣ Redirect the user to the homepage
  return sendRedirect(event, '/')
})
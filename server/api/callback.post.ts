import jwt from 'jsonwebtoken'
import fs from 'fs'
import { verifyNonce } from './auth0'
import { nanoid } from 'nanoid'

//api route handler listening for a request
export default defineEventHandler(async event => {
  const body = await readBody(event)
  //security token is given by the body
  const srtoken = body.id_token
  //verify the token. If succeeding, "claims" now has the decoded payload. Verified with public key
  const claims: any = jwt.verify(srtoken, fs.readFileSync(process.cwd()+"/cert-dev.pem"))
  // check for valid nonce claims, preventing replay attacks by making sure the nonce from "claims" wasn't used before in memories
  if(claims instanceof Object && "nonce" in claims && verifyNonce(claims["nonce"].toString())){
    //if so, now secure the token in a cookie. This way the cookie is secure and can't be stolen
    setCookie(event, 'srtoken', srtoken)
  }
  //after doing all this authentication, redirect user to main page
  await sendRedirect(event, '/')
})
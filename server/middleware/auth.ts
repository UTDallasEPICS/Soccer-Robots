import jwt from "jsonwebtoken" // used to verify JWT tokens
import { PrismaClient } from "@prisma/client" // access the database

const prisma = new PrismaClient() // create a prisma instance

export default defineEventHandler(async (event) => {
  // attach prisma to the request context so it can be used in other handlers
  event.context.prisma = prisma

  // get the 'srtoken' cookie, which should contain the jwt token
  const srtoken = getCookie(event, 'srtoken') || ''
  if (!srtoken) {
    // if no token, clear the user cookie and exit early
    setCookie(event, 'sruser', '')
    return
  }

  try {
    // verify the jwt token using the secret key
    const claims: any = jwt.verify(srtoken, process.env.JWT_SECRET!)
    // attach decoded claims to the request context
    event.context.claims = claims

    // fetch the player based on id from the token claims
    const player = await prisma.player.findUnique({
      where: { user_id: claims.id },
      select: { username: true, role: true }
    })

    if (player) {
      // if player found, store the player info as a cookie
      setCookie(event, 'sruser', JSON.stringify(player))
    } else {
      // if no player, clear user cookie
      setCookie(event, 'sruser', '')
    }
  } catch (err) {
    // if token is invalid or expired, clear cookies and log error
    console.error("JWT verification failed:", err)
    setCookie(event, 'srtoken', '')
    setCookie(event, 'sruser', '')
  }
})

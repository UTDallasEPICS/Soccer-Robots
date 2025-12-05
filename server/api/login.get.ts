import { PrismaClient } from "@prisma/client"
import { nanoid } from "nanoid"
import { getQuery, setCookie, defineEventHandler, sendRedirect } from "h3"
import jwt from "jsonwebtoken"


const prisma = new PrismaClient()

export default defineEventHandler(async (event) => {
  const { token } = getQuery(event)

  if (!token || typeof token !== "string") {
    throw createError({
      statusCode: 400,
      statusMessage: "Missing or invalid token"
    })
  }

  // Validate magic login token
  const player = await prisma.player.findFirst({
    where: {
      magicToken: token,
      tokenExpiry: { gt: new Date() }
    }
  })

  if (!player) {
    throw createError({
      statusCode: 401,
      statusMessage: "Invalid or expired magic link"
    })
  }

  // Generate permanent session token
  const payload = { id: player.user_id }
  const sessionToken = jwt.sign(payload, process.env.JWT_SECRET!, { expiresIn: '30d' })
  // Update user → remove magic link + set permanent token
  await prisma.player.update({
    where: { user_id: player.user_id },
    data: {
      magicToken: null,
      tokenExpiry: null,
      sessionToken,
    }
  })

  console.log(`Player ${player.username} logged in with session token.`)
  // Set auth cookie
  setCookie(event, "srtoken", sessionToken, {
    httpOnly: true,
    path: "/",
    maxAge: 60 * 60 * 24 * 30, // 30 days
    sameSite: "lax",
    secure: false
  })

  console.log(`Set srtoken cookie for player ${player.username}.`)
  // Set username cookie
  setCookie(event, "sruser", player.username, {
  path: "/",
  maxAge: 60 * 60 * 24 * 30,
  sameSite: "lax",
  secure: false
})

console.log(`Set sruser cookie for player ${player.username}.`) 


  // Redirect to player page
  return sendRedirect(event, "/player")
})

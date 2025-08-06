// /api/auth/login.get.ts
import { PrismaClient } from "@prisma/client"
import jwt from "jsonwebtoken"
console.log('JWT_SECRET:', process.env.JWT_SECRET)

const prisma = new PrismaClient()

export default defineEventHandler(async (event) => {
  const token = getQuery(event).token

  if (!token || typeof token !== "string") {
    throw createError({ statusCode: 400, statusMessage: "Missing or invalid token" })
  }

  const player = await prisma.player.findFirst({
    where: {
      magicToken: token,
      tokenExpiry: { gt: new Date() }
    }
  })

  if (!player) {
    throw createError({ statusCode: 401, statusMessage: "Invalid or expired token" })
  }

  // Clear token so it can't be reuse
  await prisma.player.update({
    where: { user_id: player.user_id },
    data: {
      magicToken: null,
      tokenExpiry: null
    }
  })

  const jwtToken = jwt.sign(
    {
      id: player.user_id,
      email: player.email,
      username: player.username,
      role: player.role
    },
    process.env.JWT_SECRET!,
    { expiresIn: "1h" }
  )

  // Set cookie for frontend auth session
  setCookie(event, 'srtoken', jwtToken, {
    httpOnly: true,
    path: '/',
    maxAge: 60 * 60
  })

  return { message: "Login successful", username: player.username }
})

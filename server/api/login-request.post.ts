import { PrismaClient } from "@prisma/client"
import { nanoid } from "nanoid"
import { setCookie } from "h3" 

const prisma = new PrismaClient()

export default defineEventHandler(async (event) => {
  const body = await readBody(event)
  const email = body.email

  if (!email) {
    throw createError({ statusCode: 400, statusMessage: 'Email is required' })
  }

  let player = await prisma.player.findUnique({ where: { email } })

  const token = nanoid(32)
  const expiry = new Date(Date.now() + 10 * 60 * 1000)
  
  if (!player) {
    player = await prisma.player.create({
      data: {
        user_id: nanoid(),
        email,
        username: `user-${nanoid(6)}`
      }
    })
  }

  player = await prisma.player.update({
    where: { email },
    data: {
      magicToken: token,
      tokenExpiry: expiry
    }
  })

  setCookie(event, 'magic_token', token, {
    httpOnly: true,
    path: '/',
    sameSite: 'lax',
    secure: false,
  })

console.log("player:", player);


  const loginLink = `http://localhost:3000/login?token=${token}`

  console.log('\n================ MAGIC LINK =================')
  console.log(` Email: ${email}`)
  console.log(` Link: ${loginLink}`)
  console.log('============================================\n')

  return { message: 'Magic login link has been sent (terminal).' }
})

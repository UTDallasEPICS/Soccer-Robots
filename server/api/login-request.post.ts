import { PrismaClient } from "@prisma/client"
import { nanoid } from "nanoid"

const prisma = new PrismaClient()

export default defineEventHandler(async (event) => {
  const body = await readBody(event)
  const email = body.email

  if (!email) {
    throw createError({ statusCode: 400, statusMessage: 'Email is required' })
  }

  let player = await prisma.player.findUnique({ where: { email } })

  if (!player) {
    player = await prisma.player.create({
      data: {
        user_id: nanoid(),
        email,
        username: `user-${nanoid(6)}`
      }
    })
  }

  const token = nanoid(32)
  const expiry = new Date(Date.now() + 10 * 60 * 1000)

  await prisma.player.update({
    where: { email },
    data: {
      magicToken: token,
      tokenExpiry: expiry
    }
  })

  const loginLink = `http://localhost:3000/login?token=${token}`

  console.log('\n================ MAGIC LINK =================')
  console.log(` Email: ${email}`)
  console.log(` Link: ${loginLink}`)
  console.log('============================================\n')

  return { message: 'Magic login link has been sent (terminal).' }
})

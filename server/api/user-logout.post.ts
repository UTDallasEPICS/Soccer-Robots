import { PrismaClient } from '@prisma/client'
import { defineEventHandler, getCookie, setCookie, sendRedirect } from 'h3'

const prisma = new PrismaClient()


export default defineEventHandler(async (event) => {
  const sessionToken = getCookie(event, 'srtoken')
  console.log("Logout API called")
  console.log("sessionToken:", sessionToken)
  if (sessionToken) {
    try {
      await prisma.player.update({ // or use `update` if token is unique
        where: { sessionToken },
        data: { sessionToken: null }
      })
    } catch (err) {
    }
  }

  // Clear cookies
  setCookie(event, 'srtoken', '', {
     maxAge: 0,
     httpOnly: true,
     path: '/'
    })
  setCookie(event, 'sruser', '', {
     maxAge: 0, 
     httpOnly: true, 
     path: '/' 
    })

  return sendRedirect(event, '/')
})

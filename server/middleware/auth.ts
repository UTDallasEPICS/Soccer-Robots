import jwt from "jsonwebtoken"
import fs from 'fs'
import { loginRedirectUrl } from "../api/auth0"
import { PrismaClient } from "@prisma/client"

const prisma = new PrismaClient()

//I believe this code sets the sruser cookie for the player
export default defineEventHandler(async event => {
  event.context.prisma = prisma
  //try to get security token first
  const srtoken = getCookie(event, 'srtoken') || ''
  if(srtoken){
    //try to verify the 
    try {
      //get the decoded srtoken cookie into claims
      const claims:any = jwt.verify(srtoken, fs.readFileSync(process.cwd() + '/cert-dev.pem'))
      //if claims exists and it was a number only used once, do the following
      if(claims instanceof Object && "nonce" in claims){
        event.context.claims = claims
        //get Id from the srtoken, and check if it's in prisma
        const id = claims['sub']
        const player = await prisma.player.findUnique({
          where:{
            user_id: id
          }, 
          select: {
            username: true,
            role: true
          } 
        })
        //if able to find the player, set the sruser cookie, otherwise leave blank
        if(player){
          setCookie(event, 'sruser', JSON.stringify(player))
        } else {
          setCookie(event, 'sruser', '')
        }
      }
    }
    //when fails to decode srtoken cookie. In such a case try to redirect them to login 
    catch (error) {
      console.error(error)
      setCookie(event, 'srtoken', '')
      return await sendRedirect(event, loginRedirectUrl())
    }
  }
  //if srtoken doesn't exist, set blank cookies 
  else {
    setCookie(event, 'sruser', '')
    setCookie(event, 'srtoken', '')
  }
})

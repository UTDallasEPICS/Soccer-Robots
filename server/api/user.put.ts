//this updates user's username in the database.
export default defineEventHandler(async (event) => {

  const prisma = event.context.prisma
  const claims = event.context.claims

  const body = await readBody(event)
  const username = body.username
  //checks to see if the player's NEW username is in the database
  const existingUsername = await prisma.player.findFirst({
    where: {
      username
    }
  })
  
  let msg
  //if not, can update the user's name
  if(!existingUsername){
    //now gets player with their user id, and updates their username 
    const player = await prisma.player.update({
      where: {
        user_id: claims['id'],
      },
      data: {
        username,
      },
      select: {
        username: true,
        role: true
      } 
    })
    msg = 200
    setCookie(event, 'sruser', JSON.stringify(player))
  } else {
    //otherwise if such a username already exists, update fails.
    msg = 403
  }

  return msg
})

// 403 - username is not unique
// 200 went okay
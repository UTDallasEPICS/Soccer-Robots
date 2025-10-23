//gets the user data.
export default defineEventHandler(async (event) => {
  const prisma = event.context.prisma
  const claims = event.context.claims
  const body = await readBody(event)
  const username = body.username
  //finds if the user is already in the database.
  const existingUsername = await prisma.player.findFirst({
    where: {
      username
    }
  })
  
  //will be if it succeeded or not
  let msg
  //if they aren't in the database, put them in the database
  if(!existingUsername){
    //create player in database with their username, unique user_id, and email. Returns the username and role fields of the user.
    console.log("Claims:", claims);
    const player = await prisma.player.create({
      data: {
        user_id: claims['id'],
        username,
        email: claims['email']
      },
      select: {
        username: true,
        role: true
      } 
    })
    msg = 200
    //now set cookie with user's data 
    setCookie(event, 'sruser', JSON.stringify(player))
  } else {
    msg = 403
  }

  return msg
})

// 403 - username is not unique
// 200 went okay
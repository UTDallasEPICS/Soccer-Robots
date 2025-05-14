import { PrismaClient, Player } from '@prisma/client'

//listens for event to get the leaderboard. It fetches the top 5 chars from the leaderboard
export default defineEventHandler(async (event) => {
  const prisma = event.context.prisma
  //gets which column is going to be sorted.
  const { sortedColumn } = getQuery(event)

  //order them by descending order
  const allowedColumns = ['wins', 'goals', 'losses', 'ratio', 'username'] as const

  const column = allowedColumns.includes(sortedColumn as any)
    ? (sortedColumn as string)
    : 'wins' 
  
  //now take first 5 from the column that's being sorted and put that as the player data. For example of the column from event was "score", gets top 5 players
  //with the highest scores
  const playerData = await prisma.player.findMany({
    orderBy: {
      [column]: 'desc',
    },
    take: 5,
  }) as Player[]

  return playerData
})
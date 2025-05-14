import { PrismaClient, Player } from '@prisma/client'

export default defineEventHandler(async (event) => {
  const prisma = event.context.prisma
  const { sortedColumn } = getQuery(event)

  const allowedColumns = ['wins', 'goals', 'losses', 'ratio', 'username'] as const

  const column = allowedColumns.includes(sortedColumn as any)
    ? (sortedColumn as string)
    : 'wins' 

  const playerData = await prisma.player.findMany({
    orderBy: {
      [column]: 'desc',
    },
    take: 5,
  }) as Player[]

  return playerData
})
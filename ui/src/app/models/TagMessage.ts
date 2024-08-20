import {ReceiverId} from "./receiver";

export interface TagMessage {
  uuid: ReceiverId,
  rssi: number,
  addr: string,
  time_sent: number,
  msg_counter: number
}

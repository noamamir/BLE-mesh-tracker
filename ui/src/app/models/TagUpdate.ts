import {ReceiverId} from "./receiver";
import {Beacon} from "./beacon";

export interface TagUpdate {
  receiver: ReceiverId,
  beacon: Beacon,
  serial: number
}

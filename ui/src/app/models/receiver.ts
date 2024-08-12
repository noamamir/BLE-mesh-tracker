import {Beacon, BeaconId} from "./beacon";

export interface Receiver {
  uuid: ReceiverId,
  beacons: Record<BeaconId, Beacon>
}

export type ReceiverId = string

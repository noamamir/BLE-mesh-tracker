import {Beacon, BeaconId} from "./beacon";

export interface Receiver {
  id: ReceiverId,
  beacons: Record<BeaconId, Beacon>;
}

export type ReceiverId = string

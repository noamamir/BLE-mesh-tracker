export interface Beacon {
  ID: BeaconId,
  rssi: number,
  lastUpdated: number
}

export type BeaconId = string

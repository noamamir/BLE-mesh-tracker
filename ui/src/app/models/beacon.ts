
export type BeaconId = string

// models/beacon.model.ts
export class Beacon {
  constructor(
    public ID: BeaconId,
    public rssi: number,
    public lastUpdated: number
  ) {}

  updateBeacon(rssi: number): void {
    this.rssi = rssi;
    this.lastUpdated = Date.now();
  }
}

import {Injectable, NgZone} from '@angular/core';
import {Beacon} from "../../models/beacon";
import {TagUpdate} from "../../models/TagUpdate";
import {interval, map, Subject} from "rxjs";
import {getSerial} from "../heartbeat/heart-beat.service";

@Injectable({
  providedIn: 'root'
})
export class TagUpdatesService {

  updates = new Subject<TagUpdate[]>()

  constructor(private ngZone: NgZone) {
    this.ngZone.runOutsideAngular(() => {
      interval(1000).pipe(
        map(() => Array.from({length: 3}, _ => this.generateTag())),
      )
        .subscribe(this.updates)
    })
  }


  generateTag(): TagUpdate {
    const receiver = generateReceiverName()
    const serial = getSerial(receiver)
    return {
      receiver,
      beacon: generateBeacon(),
      serial
    }
  }


}

export function randomInt(min: number, max: number) {
  min = Math.ceil(min);
  max = Math.floor(max);
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

export function generateBeacon(): Beacon {
  return {
    ID: generateBeaconName(),
    lastUpdated: Date.now(),
    rssi: randomInt(0, 100)
  }
}

export function generateBeaconName() {
  return `be00${randomInt(1, 5)}`
}

export function generateReceiverName() {
  return `rec00${randomInt(1, 5)}`
}

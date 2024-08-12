import {Pipe, PipeTransform} from '@angular/core';
import {State} from "../services/grid-state/grid-state.service";
import {ReceiverId} from "../models/receiver";
import {Beacon, BeaconId} from "../models/beacon";

@Pipe({
  name: 'beaconAtReceiver',
  pure: true
})
export class BeaconAtReceiverPipe implements PipeTransform {

  transform(state: State | null, receiverName: ReceiverId, beaconName: BeaconId): Beacon | null {
    if (!state) return null
    return state[receiverName]?.beacons[beaconName]
  }

}

import {Pipe, PipeTransform} from '@angular/core';
import {ReceiverId} from "../models/receiver";

@Pipe({
  name: 'isReceiverInactive',
})
export class IsReceiverInactivePipe implements PipeTransform {

  transform(receiver: ReceiverId, active: Set<ReceiverId> | null): boolean {
    if (active == null) {
      console.log('its null!')
      return true
    }
    console.log(receiver,!active.has(receiver))
    return !active.has(receiver)
  }
}
